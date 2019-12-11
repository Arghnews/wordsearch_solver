#include "wordsearch_solver.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iterator>
#include <memory>
#include <ostream>
#include <type_traits>
#include <tuple>
#include <utility>
#include <set>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h> // Need for printing vector etc. with prettyprint
#include "spdlog/spdlog.h"
#include "prettyprint.hpp"
#include "jr_assert/jr_assert.h"
#include "dictionary.h"

#include <gperftools/profiler.h>
#include "nonstd/span.hpp"
// #include <boost/dynamic_bitset/dynamic_bitset.hpp>

// For now for debug
#include "trie.h"
#include "wordsearch_solver_defs.h"

// #define //PRINT fmt::print
// #define //PRINT(...)
// #define //PRINT //

/* template<class T1, class T2> */
/* std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& i) */
/* { */
/*   return os << "{" << i.first << ", " << i.second << "}"; */
/* } */

/* template <> */
/* struct fmt::formatter<wordsearch_solver::Index> { */
/*   template <typename ParseContext> */
/*   constexpr auto parse(ParseContext &ctx) { return ctx.begin(); } */

/*   template <typename FormatContext> */
/*   auto format(const wordsearch_solver::Index& d, FormatContext &ctx) { */
/*     /1* return format_to(ctx.out(), "{}-{}-{}", d.year, d.month, d.day); *1/ */
/*     return format_to(ctx.out(), "{{{}, {}}}", d.first, d.second); */
/*   } */
/* }; */

namespace
{

using namespace wordsearch_solver;

// Output parameters make me sad but returning a vector by value and the ensuing
// copying was slow
nonstd::span<Index, nonstd::dynamic_extent>
surrounding(
    const std::size_t i, const std::size_t j, const Grid& gridp,
    nonstd::span<Index, 8> result)
    // const std::array<Index, 8>& arr)
{
  const auto& grid = *gridp;

  // const auto possible_directions = 8ULL;
  // result.resize(possible_directions);
  std::size_t result_size = 0;
  auto last_result = result.begin();
  const auto insert_if_valid = [&last_result, &grid, &result_size]
    (const auto i_, const auto j_)
    {
      // Could use limits max type, this works for signed too, for now size_t
      // anyway
      const std::size_t lower_lim = -1ULL;
      if (i_ != lower_lim && j_ != lower_lim && i_ < grid.size() &&
          j_ < grid[i_].size())
      /* i < grid.size() && j < grid.at(i).size()) */
      {
        last_result->first = i_;
        last_result->second = j_;
        ++last_result;
        ++result_size;
      }
  };

  // Now arranged so they will be sorted as pairs
  /* {i - 1, j - 1} // NW */
  /* {i - 1, j} // N */
  /* {i - 1, j + 1} // NE */

  /* {i, j - 1} // W */
  /* {i, j + 1}, // E */

  /* {i + 1, j - 1} // SW */
  /* {i + 1, j} // S */
  /* {i + 1, j + 1}, // SE */

  insert_if_valid(i - 1, j - 1);
  insert_if_valid(i - 1, j);
  insert_if_valid(i - 1, j + 1);

  insert_if_valid(i, j - 1);
  insert_if_valid(i, j + 1);

  insert_if_valid(i + 1, j - 1);
  insert_if_valid(i + 1, j);
  insert_if_valid(i + 1, j + 1);

  return result.first(result_size);
  // result.resize(
      // static_cast<std::size_t>(std::distance(result.begin(), last_result)));
  // for (auto c = std::distance(last_result, result.end()); c > 0; --c)
  // {
    // result.pop_back();
  // }
}

template<class It1, class It2>
constexpr bool contains_no_duplicates(It1 it1, It2 it2)
{
  return std::adjacent_find(it1, it2) == it2;
}

template<class Container>
constexpr bool contains_no_duplicates(Container& c)
{
  // Heavyweight but meh
  std::vector<typename Container::value_type> cont(
      std::begin(c), std::end(c));
  std::sort(cont.begin(), cont.end());
  return contains_no_duplicates(cont.begin(), cont.end());
}

// As the name says. Finds the set difference - what is in the second range that
// is not in the first - and writes it into the second range. Both ranges must
// contain no duplicates.
template<class Iter1, class Iter2>
Iter2 remove_from_second_if_in_first(
    Iter1 ti,
    const Iter1 ti_end,
    Iter2 ni,
    const Iter2 ni_end
    )
{
  JR_ASSERT(contains_no_duplicates(ni, ni_end), "Input must be unique");
  JR_ASSERT(contains_no_duplicates(ti, ti_end), "Input must be unique");
  return std::remove_if(ni, ni_end,
      [ti, ti_end] (const auto& val)
      {
        return std::find(ti, ti_end, val) != ti_end;
      });
}

void result_inserts(
    const Result& result,
    // const Indexes& suffixes,
    const nonstd::span<Index, nonstd::dynamic_extent> suffixes,
    StringIndexes& stringindexes,
    const std::string& suffixes_str,
    const Indexes& tail_indexes,
    const std::string& tail_word,
    Indexes& new_layer
    )
{
    const auto concat = [] (std::vector<Index> indexes, const Index index)
    {
      indexes.push_back(index);
      return indexes;
    };

    for (const auto i: result.contains)
    {
      stringindexes.insert(
          {tail_word + suffixes_str[i], concat(tail_indexes, suffixes[i])});
    }
    for (const auto i: result.contains_and_further)
    {
      stringindexes.insert(
          {tail_word + suffixes_str[i], concat(tail_indexes, suffixes[i])});
      new_layer.emplace_back(suffixes[i]);
    }
    for (const auto i: result.further)
    {
      new_layer.emplace_back(suffixes[i]);
    }
}

/* Sadly from looking at a call graph it seems that the std::string_view is not
 * optimised out whereas passing needle by const& is */
/* template<class Container> */
//bool vec_contains_string_that_starts_with(
//    const Dictionary& dictionary,
//    /* const Container& haystack, */
//    const std::string& needle)
//    /* const std::string_view needle) */
//{

} // namespace

template<>
struct fmt::formatter<wordsearch_solver::Index>
{
  constexpr auto parse(format_parse_context& ctx)
  {
    auto it = ctx.begin(), end = ctx.end();
    // if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const wordsearch_solver::Index& p, FormatContext& ctx)
  {
    // ctx.out() is an output iterator to write to.
    return format_to(ctx.out(), "{{{}, {}}}", p.first, p.second);
  }

};

template<>
struct fmt::formatter<nonstd::span<Index, nonstd::dynamic_extent>>
{
  constexpr auto parse(format_parse_context& ctx)
  {
    auto it = ctx.begin(), end = ctx.end();
    // if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const nonstd::span<Index, nonstd::dynamic_extent>& p,
      FormatContext& ctx)
  {
    // ctx.out() is an output iterator to write to.
    return format_to(ctx.out(), "{}", fmt::join(p.begin(), p.end(), ", "));
  }

};

namespace wordsearch_solver
{

// NOTE: do not move this to detail (like you've tried at least twice now) as
// it's used in at least one place outside this translation unit.
std::string indexes_to_word(const Grid& grid,
    const nonstd::span<Index, nonstd::dynamic_extent> tail)
{
  std::string word;
  word.reserve(tail.size());
  // spdlog::debug("Grid: {}", grid);
  for (const auto [i, j]: tail)
  {
    // spdlog::debug("Index [{}, {}]", i, j);
    // word.push_back(grid->at(i).at(j));
    word.push_back((*grid)[i][j]);
  }
  return word;
}

    // Ok so.
    // Currently (especially with a single trie like data structure) we do extra
    // work.
    // Current method is to see if contains this word, then add surrounding
    // words in the wordsearch ie. this word with suffixes around it if these
    // prefixes have further entries. Any that have further entries then get
    // added to the next layer.
    // However in this, for stuff like the trie at least and for other impls,
    // when we find the prefixes for the next layer to add, we do all the work
    // to find out if the next layer items are contained too. And then currently
    // we then add them to the next layer, and then on said next layer we will
    // once again check if they are contained and then check if prefix entries
    // exist with the new suffixes.
    // My current solution has a bool attached to each layer entry in "a". This
    // says whether or not this entry has been processed?
    // If bool is true, has been added already and don't need to do contains on
    // it, just contains_prefix on the new suffixes.
    // If bool is false (default), then we act as normal checking if it's
    // contained and then checking adding surrounding letters.
    // This may also help if decided to try a split impl with a hash table for
    // contains and a contains_prefix trie or other (although tbh unlikely this
    // would be faster).
    //
    //
    //
    //
    //
    // !!!!!!!!!!!!!!!!!!! -> Change to array of bools as well passing
    // suffixes etc. This keeps position too so that index is easy to find.
    //
    //
    //
    // 4 cases.
    // contained and further
    // contained but no further
    // further but not contained
    // neither
    //
    // if unchecked(tail_word)
    //   valid_suffixes = contains_prefixes(tail_word, suffixes)
    //   for (const auto c: valid_suffixes)
    //     new_layer.push_back(tail_word + valid_suffixes)
    //  else
    //   [valid_suffixes, ] = contains_and_prefixes(tail_word, suffixes)
    //   for (const auto c: valid_suffixes)
    //
    //

/* Several points - for the find_words major loop we don't need to start at
 * dictionary.begin() every time if remembered where we were in dictionary - or
 * even if we stored all the iters from where we were so could go to them when
 * popped back. */
/* For back of the search let's just try a simple index on the next letter as
 * the end point. Then should go and see what the usual distances are. */

//std::pair<std::vector<std::string>, std::vector<std::vector<Index>>>
// __attribute__((__noinline__))
StringIndexes find_words(
    const Dictionary& dictionary, const Grid& grid, const Index start)
//    StringIndexes& stringindexes)
{
  //
  //
  //
  //
  // TODO: refactor the shit out of this
  //
  //
  //
  //

  JR_ASSERT(!grid->empty());
  std::vector<std::string> found_words{};
  std::vector<std::vector<Index>> found_indexes{};
  std::vector<std::vector<Index>> a{};
  std::array<Index, 8> next_indexes{};

  std::vector<Index> tail_indexes{};
  std::string tail_word{};

  StringIndexes stringindexes{grid};

  const auto index_to_char = [&grid] (const auto index)
  {
    /* return grid.at(index.i).at(index.j); */
    return grid->at(index.first).at(index.second);
    // return (*grid)[index.first][index.second];
  };

  // Same treatment as next_indexes as this can only ever be up to size 8
  // Perhaps replace both with boost::small_vector
  // In fact should be able to do this for both
  std::vector<Index> new_layer;

  auto push_back_on_tail = [&](const Index index)
  {
    tail_indexes.push_back(index);
    tail_word.push_back(index_to_char(index));
    JR_ASSERT(contains_no_duplicates(tail_indexes), "Added {}", index);
  };

  auto pop_back_off_tail = [&]()
  {
    JR_ASSERT(!tail_indexes.empty());
    tail_indexes.pop_back();
    tail_word.pop_back();
  };

  {
    const std::vector<Index> init_suffixes = {start};
    const auto init_suffixes_str = std::string{index_to_char(start)};
    wordsearch_solver::Result init_result;
    dictionary.contains_and_further("", {index_to_char(start)}, init_result);

    // Output words that satisfy contains, add those that satisfy further to the
    // queue to be added next iteration
    for (const auto i: init_result.contains)
    {
      stringindexes.insert({std::string{init_suffixes_str[i]}, init_suffixes,
          grid});
    }
    for (const auto i: init_result.contains_and_further)
    {
      stringindexes.insert({std::string{init_suffixes_str[i]}, init_suffixes,
          grid});
      new_layer.emplace_back(init_suffixes[i]);
    }
    for (const auto i: init_result.further)
    {
      new_layer.emplace_back(init_suffixes[i]);
    }

    if (!new_layer.empty())
    {
      // spdlog::debug/("Adding new layer of letters {}", indexes_to_word(grid,
  //            new_layer));
      push_back_on_tail(new_layer[0]);
      a.emplace_back(std::move(new_layer));
    }
  }

  ProfilerEnable();

  wordsearch_solver::Result result;

  while (!a.empty())
  {
    new_layer.clear();
    //PRINT("\n");
    // spdlog::debug/("\nIteration");
    JR_ASSERT(!a.empty() && !a.back().empty());
    const auto& last = a.back().front();

    // spdlog::debug/("Tail is {}", tail_word);
    // spdlog::debug/("At position {} {}", last, index_to_char(last));

    // Have put in template args for now as want to experiment/ensure that
    // static extent is used, see if it brings any benefit/downside
    auto surrounding_indexes = surrounding(last.first, last.second, grid,
          nonstd::span<Index, 8>(next_indexes));

    JR_ASSERT(contains_no_duplicates(surrounding_indexes));

    //PRINT("tail_indexes: {}\n", tail_indexes);
    //PRINT("surrounding_indexes: {}\n",
        // fmt::format("{}", fmt::join(surrounding_indexes.begin(),
          // surrounding_indexes.end(),
          // ", "))
        // );
    const auto surround_end_it = std::remove_if(surrounding_indexes.begin(),
              surrounding_indexes.end(),
              [&] (const auto& index)
              {
                return std::find(tail_indexes.begin(), tail_indexes.end(),
                    index) != tail_indexes.end();
              });
    surrounding_indexes = surrounding_indexes.first(
        static_cast<std::size_t>(
          std::distance(surrounding_indexes.begin(), surround_end_it)
          )
        );
    //PRINT("surrounding_indexes after first: {}\n", surrounding_indexes);

    JR_ASSERT(contains_no_duplicates(surrounding_indexes));

    const auto& suffixes = surrounding_indexes;
    const std::string suffixes_str = indexes_to_word(grid, suffixes);

    dictionary.contains_and_further(tail_word, suffixes_str, result);

    result_inserts(
        result,
        suffixes,
        stringindexes,
        suffixes_str,
        tail_indexes,
        tail_word,
        new_layer);

    result.clear();

    if (!new_layer.empty())
    {
      // spdlog::debug/("Adding new layer of letters {}", indexes_to_word(grid,
//            new_layer));

      //PRINT("!new_layer.empty() push back of {}\n", new_layer);
      push_back_on_tail(new_layer[0]);
      a.emplace_back(std::move(new_layer));
    } else
    {
      // spdlog::debug/("Popping a.back().front() as no letters");
      // spdlog::debug/("Losing {}", last);
      // spdlog::debug/("a: {}", a);
      while (!a.empty() && a.back().size() <= 1)
      {
        // spdlog::debug/("Popping additional size 1 {}", a.back());
        a.pop_back();
        pop_back_off_tail();
      }
      if (!a.empty())
      {
        // spdlog::debug/("Popping last of back {}", a.back().front());
        // spdlog::debug/("tail_indexes: {} vs a: {} vs a.back(): {}",
        // tail_indexes, a, a.back());
        a.back().erase(a.back().begin());
        pop_back_off_tail();
        if (!a.back().empty())
        {
          //PRINT("Lower push back of {}\n", a.back().front());
          push_back_on_tail(a.back().front());
        }
        // spdlog::debug/("after ops: tail_indexes: {} vs a: {} vs a.back(): {}",
        // tail_indexes, a, a.back());
      }
      // spdlog::debug/("a: {}", a);

      /* assert(!tail_indexes.empty()); */
    }
  }

//  return {found_words, found_indexes};
  ProfilerDisable();
  return stringindexes;
}

Grid grid_from_file(const std::filesystem::path& wordsearch_file)
{
  namespace fs = std::filesystem;
  /* //PRINT("Reading wordsearch from file\n"); */
  spdlog::debug("Reading wordsearch from file");
  /* fs::path wordsearch_file{argv[1]}; */
  // FIXME: have just noticed my clangd is flagging this as passing non-POD
  // type to varargs function which stackoverflow tells me is implementation
  // defined. However the varargs function captures by forwarding reference not
  // by value, unsure if this is ok or not?
  // Calling c_str() for now
  JR_ASSERT(fs::exists(wordsearch_file), "Wordsearch file must exist at path {}",
      wordsearch_file.c_str());
  auto grid = std::make_shared<Grid::element_type>();
  // Interesting idea - https://stackoverflow.com/a/18514815/8594193
  std::ifstream inp{wordsearch_file};
  std::string line;
  for (; std::getline(inp, line);)
  {
    /* Erase every other character, keeping the first */
    bool char_next = true;
    line.erase(std::remove_if(line.begin(), line.end(),
          [&char_next] (const auto)
          {
            return char_next = !char_next;
          }), line.end());
    for (auto& c: line)
    {
      c = static_cast<char>(std::tolower(c));
    }
    grid->emplace_back(std::move(line));
    /* //PRINT("Line: {}\n", line); */
  }

  JR_ASSERT(!grid->empty(), "Read empty grid from file: {}",
      wordsearch_file.c_str());
  // Left check in here so if prior line deleted/assert turned off this
  // won't yield UB.
  if (!grid->empty())
  {
    const auto size = grid->begin()->size();
    for (const auto& row: *grid)
    {
      JR_ASSERT(row.size() == size, "Expected rectangular grid but "
                "row sizes differ: {} vs {}", row.size(), size);
    }
  }

  return grid;
}

/* To get just the list of words flatten the .first returned vector  */
//std::pair<
//  std::vector<std::vector<std::string>>,
//  std::vector<std::vector<std::vector<Index>>>
//>
StringIndexes solve(const Dictionary& dict, const Grid& grid)
{
  std::vector<std::vector<std::string>> words_found;
  std::vector<std::vector<std::vector<Index>>> list_of_indexes_found;

  StringIndexes stringindexes{grid};

  /* More generic, less readable?.. */
  using std::begin;
  using std::end;
  for (auto i_it = begin(*grid); i_it != end(*grid); std::advance(i_it, 1))
  {
    for (auto j_it = begin(*i_it); j_it != end(*i_it); std::advance(j_it, 1))
    {
      stringindexes.concat(find_words(dict, grid, Index{
          std::distance(begin(*grid), i_it),
          std::distance(begin(*i_it), j_it),
          }));
//      find_words(dict, grid, Index{
//          std::distance(begin(*grid), i_it),
//          std::distance(begin(*i_it), j_it),
//          }, stringindexes);
//      words_found.push_back(std::move(words));
//      list_of_indexes_found.push_back(std::move(indexes));
    }
  }
//  return {words_found, list_of_indexes_found};
  return stringindexes;
}

//sortable
//to_string() / operator std::string()

} // namespace wordsearch_solver
