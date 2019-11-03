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
  spdlog::debug("Grid: {}", grid);
  for (const auto [i, j]: tail)
  {
    spdlog::debug("Index [{}, {}]", i, j);
    word.push_back(grid->at(i).at(j));
  }
  return word;
}

/* Several points - for the find_words major loop we don't need to start at
 * dictionary.begin() every time if remembered where we were in dictionary - or
 * even if we stored all the iters from where we were so could go to them when
 * popped back. */
/* For back of the search let's just try a simple index on the next letter as
 * the end point. Then should go and see what the usual distances are. */

//std::pair<std::vector<std::string>, std::vector<std::vector<Index>>>
__attribute__((__noinline__)) StringIndexes find_words(
    const Dictionary& dictionary, const Grid& grid, const Index start)
{
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

//  stringindexes.insert()
  a.push_back({start});
  tail_indexes.push_back({start});
  tail_word.push_back(index_to_char(start));

  while (!a.empty())
  {
    // spdlog::debug/("\nIteration");
    JR_ASSERT(!a.empty() && !a.back().empty());
    const auto& last = a.back().front();

    // spdlog::debug/("Tail is {}", tail_word);
    // spdlog::debug/("At position {} {}", last, index_to_char(last));

    const auto tail_word_opt = dictionary.find(tail_word);
//    const auto tail_word_it = lower_bound(dictionary, tail_word);
    if (tail_word_opt && !(tail_word < **tail_word_opt))
    /* if (test_contains(dictionary, tail_word)) */
    {
      // spdlog::debug/("Outputting: {}", tail_word);
//      found_words.push_back(tail_word);
//			found_indexes.push_back(tail_indexes);
      stringindexes.insert({grid, tail_word, tail_indexes});
      JR_ASSERT(tail_word.size() == tail_indexes.size());
    }

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
    std::vector<Index> new_layer;
    const auto possible_directions = 8ULL;
    new_layer.reserve(possible_directions);

    /* This last char should be changed every loop iteration */
    tail_word.push_back('\0');
    for (const auto [i, j]: next_indexes)
    {
      const char letter = grid->at(i).at(j);
      tail_word.back() = letter;
      static_assert(std::is_same_v<
          std::remove_cv_t<
          std::remove_reference_t<decltype(tail_word)>>, std::string>);
      // spdlog::debug/("tail_word is {}", tail_word);
      /* if (!vec_contains_string_that_starts_with(dictionary, tail_word)) */
      if (!dictionary.contains_prefix(tail_word, tail_word_opt))
//      if (!vec_contains_string_that_starts_with(dictionary, tail_word))
      {
        // spdlog::debug/("Rejecting word prefix {}", tail_word);
      } else
      {
        new_layer.emplace_back(i, j);
      }
    }
    /* Remove extra trailing char */
    tail_word.pop_back();

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
  for (auto [inp, line] =
      std::tuple{std::ifstream{wordsearch_file}, std::string{}};
      std::getline(inp, line);)
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
