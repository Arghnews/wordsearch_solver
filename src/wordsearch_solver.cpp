#include "wordsearch_solver.h"

#include <algorithm>
#include <filesystem>
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
#include "jr_assert.h"
#include "dictionary.h"

// For now for debug
#include "trie.h"

/* std::ostream& operator<<(std::ostream& os, const wordsearch_solver::Index& i) */
/* { */
/*   return os << "{" << i.first << ", " << i.second << "}"; */
/* } */

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
void surrounding(const std::size_t i, const std::size_t j,
    const Grid& gridp, std::vector<Index>& result)
{
  const auto& grid = *gridp;
  /* assert(i <= i_size); */
  /* assert(j <= j_size); */

  /* boost::container::small_vector<Index, 8> result; */
  /* result.reserve(8); */

  /* {i, j + 1}, // E */
  /* {i + 1, j + 1}, // SE */
  /* {i + 1, j} // S */

  /* {i + 1, j - 1} // SW */
  /* {i, j - 1} // W */
  /* {i - 1, j - 1} // NW */

  /* {i - 1, j} // N */
  /* {i - 1, j + 1} // NE */

  const auto possible_directions = 8ULL;
  result.resize(possible_directions);
  auto last_result = result.begin();
  const auto insert_if_valid = [&last_result, &grid]
    (const auto i_, const auto j_)
    {
      // Could use limits max type, this works for signed too, for now size_t
      // anyway
      const std::size_t lower_lim = -1UL;
      if (i_ != lower_lim && j_ != lower_lim && i_ < grid.size() &&
          j_ < grid[i_].size())
      /* i < grid.size() && j < grid.at(i).size()) */
      {
        last_result->first = i_;
        last_result->second = j_;
        ++last_result;
      }
  };

  insert_if_valid(i, j + 1);
  insert_if_valid(i + 1, j + 1);
  insert_if_valid(i + 1, j);

  insert_if_valid(i + 1, j - 1);
  insert_if_valid(i, j - 1);
  insert_if_valid(i - 1, j - 1);

  insert_if_valid(i - 1, j);
  insert_if_valid(i - 1, j + 1);

  for (auto c = std::distance(last_result, result.end()); c > 0; --c)
  {
    result.pop_back();
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

////  const auto it = std::lower_bound(first, last, needle);
//  const auto it = dictionary.lower_bound(needle);
//  /* const auto it = std::find(first, last, needle); */
//  // Hacked for now for std::set
//  // Well this sucks
//  // Requires std::set<T, std::less<>>
//  /* const auto it = haystack.lower_bound(needle); */
//  /* const auto it = haystack.lower_bound(std::string{needle}); */
//  /* Avoids using substr on needle - needle was string_view but time being
//   * wasted calling operator string on the view for substr, this (hopefully!)
//   * avoids any extraneous string */
//  /* if (it == haystack.end()) return false; */
//  if (it == dictionary.end()) return false;
//  const auto& haystack_string = *it;
//  const auto size = static_cast<
//    std::decay_t<decltype(needle)>::difference_type>(
//      std::min(haystack_string.size(), needle.size()));
//  return std::equal(
//      haystack_string.begin(), std::next(haystack_string.begin(), size),
//      needle.begin(), std::next(needle.begin(), size));
//}

//// Here for clear profiling output
//__attribute__((__noinline__)) auto lower_bound(const Dictionary& dictionary, const std::string& tail_word)
//{
////  return dictionary.lower_bound(tail_word);
//  return dictionary.find(tail_word);
////  return std::lower_bound(dictionary.begin(), dictionary.end(), tail_word);
//}

} // namespace

namespace wordsearch_solver
{

std::string indexes_to_word(const Grid& grid, const Indexes& tail)
{
  std::string word;
  word.reserve(tail.size());
  // spdlog::debug("Grid: {}", grid);
  for (const auto [i, j]: tail)
  {
    // spdlog::debug("Index [{}, {}]", i, j);
    word.push_back(grid->at(i).at(j));
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
__attribute__((__noinline__))
StringIndexes find_words(
    const Dictionary& dictionary, const Grid& grid, const Index start)
//    StringIndexes& stringindexes)
{
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  // TODO: refactor the shit out of this
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  std::vector<std::string> found_words{};
  std::vector<std::vector<Index>> found_indexes{};
  std::vector<std::vector<Index>> a{};
  std::vector<Index> next_indexes{};

  std::vector<Index> tail_indexes{};
  std::string tail_word{};

  StringIndexes stringindexes{grid};

  const auto index_to_char = [&grid] (const auto index)
  {
    /* return grid.at(index.i).at(index.j); */
    return grid->at(index.first).at(index.second);
  };

  std::vector<Index> new_layer;

//  stringindexes.insert()
  // a.push_back({start});
  // tail_indexes.push_back({start});
  // tail_word.push_back(index_to_char(start));

  // TODO: for god's sake put this in a function
  // surrounding(start.first, start.second, grid, next_indexes);
  // const auto init_suffixes = next_indexes;
  // const std::string init_suffixes_str = indexes_to_word(grid, init_suffixes);
  const std::vector<Index> init_suffixes = {start};
  const auto init_suffixes_str = std::string{index_to_char(start)};
  const wordsearch_solver::Result init_result = dictionary.contains_and_further(
      "", {index_to_char(start)});

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
    tail_indexes.push_back(new_layer[0]);
    tail_word.push_back(index_to_char(new_layer[0]));
    a.emplace_back(std::move(new_layer));
  }


  while (!a.empty())
  {
    new_layer.clear();
    // spdlog::debug/("\nIteration");
    JR_ASSERT(!a.empty() && !a.back().empty());
    const auto& last = a.back().front();

    // spdlog::debug/("Tail is {}", tail_word);
    // spdlog::debug/("At position {} {}", last, index_to_char(last));

    surrounding(last.first, last.second, grid, next_indexes);
    // spdlog::debug/("Surrouding are {} {}", next_indexes, indexes_to_word(grid, next_indexes));

    /* Remove surrounding indexes that would bite tail_indexes */
    next_indexes.erase(std::remove_if(next_indexes.begin(), next_indexes.end(),
        [&tail_indexes] (const auto& val)
        {
          return std::find(tail_indexes.begin(),
              tail_indexes.end(), val) != tail_indexes.end();
        }), next_indexes.end());
    // spdlog::debug/("Surrouding are now {}", next_indexes);

    /* Remove surrounding indexes if would not ever form word */

    const auto suffixes = next_indexes;
    const std::string suffixes_str = indexes_to_word(grid, suffixes);

    const wordsearch_solver::Result result = dictionary.contains_and_further(
        tail_word, suffixes_str);

    // Output words that satisfy contains, add those that satisfy further to the
    // queue to be added next iteration
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

    if (!new_layer.empty())
    {
      // spdlog::debug/("Adding new layer of letters {}", indexes_to_word(grid,
//            new_layer));
      tail_indexes.push_back(new_layer[0]);
      tail_word.push_back(index_to_char(new_layer[0]));
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
        tail_indexes.pop_back();
        tail_word.pop_back();
      }
      if (!a.empty())
      {
        // spdlog::debug/("Popping last of back {}", a.back().front());
        // spdlog::debug/("tail_indexes: {} vs a: {} vs a.back(): {}", tail_indexes, a, a.back());
        /* a.back().pop_front(); */
        /* Potential optim further here with faster erase that doesn't preserve
         * a nice ordering like this */
        a.back().erase(a.back().begin());
        if (!a.back().empty())
        {
          tail_indexes.back() = a.back().front();
          tail_word.back() = index_to_char(a.back().front());
        } else
        {
          tail_indexes.pop_back();
          tail_word.pop_back();
        }
        // spdlog::debug/("after ops: tail_indexes: {} vs a: {} vs a.back(): {}", tail_indexes, a, a.back());
      }
      // spdlog::debug/("a: {}", a);

      /* assert(!tail_indexes.empty()); */
    }
  }

//  return {found_words, found_indexes};
  return stringindexes;
}

Grid grid_from_file(const std::filesystem::path& wordsearch_file)
{
  namespace fs = std::filesystem;
  /* fmt::print("Reading wordsearch from file\n"); */
  spdlog::debug("Reading wordsearch from file");
  /* fs::path wordsearch_file{argv[1]}; */
  JR_ASSERT(fs::exists(wordsearch_file), "Wordsearch file must exist at path {}",
      wordsearch_file);
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
    /* fmt::print("Line: {}\n", line); */
  }

  JR_ASSERT(!grid->empty(), "Read empty grid from file: {}", wordsearch_file);
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
