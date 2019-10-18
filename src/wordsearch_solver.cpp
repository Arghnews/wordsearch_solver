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
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h> // Need for printing vector etc. with prettyprint
#include "spdlog/spdlog.h"
#include "prettyprint.hpp"
#include "jr_assert.h"

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

template <class Indexes>
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

/* Sadly from looking at a call graph it seems that the std::string_view is not
 * optimised out whereas passing needle by const& is */
/* template<class Container> */
template<class Iterator>
bool vec_contains_string_that_starts_with(
    const Iterator first,
    const Iterator last,
    /* const Container& haystack, */
    const std::string& needle)
    /* const std::string_view needle) */
{

  const auto it = std::lower_bound(first, last, needle);
  /* const auto it = std::find(first, last, needle); */
  // Hacked for now for std::set
  // Well this sucks
  // Requires std::set<T, std::less<>>
  /* const auto it = haystack.lower_bound(needle); */
  /* const auto it = haystack.lower_bound(std::string{needle}); */
  /* Avoids using substr on needle - needle was string_view but time being
   * wasted calling operator string on the view for substr, this (hopefully!)
   * avoids any extraneous string */
  /* if (it == haystack.end()) return false; */
  if (it == last) return false;
  const auto& haystack_string = *it;
  const auto size = static_cast<
    std::decay_t<decltype(needle)>::difference_type>(
      std::min(haystack_string.size(), needle.size()));
  return std::equal(
      haystack_string.begin(), std::next(haystack_string.begin(), size),
      needle.begin(), std::next(needle.begin(), size));
}

}

namespace wordsearch_solver
{

class StringIndex
{
public:
  StringIndex(const Grid& grid, std::string string,
              Indexes indexes)
    : string_(std::move(string)), indexes_(std::move(indexes))
  {
    JR_ASSERT(string_.size() == indexes_.size());
    JR_ASSERT(string_ == indexes_to_word(grid, indexes_));
  }

  std::size_t size() const
  {
    return string_.size();
  }

  const std::string& string() const
  {
    return string_;
  }

  // This feels and is used in a leaky implementation detail kinda way.
  // But the "proper" way I suspect is making iterators and bla bla and seems
  // a lot of boilerplate to accomplish the same thing and is unneeded here.
  const Indexes& indexes() const
  {
    return indexes_;
  }

private:
  std::string string_;
  Indexes indexes_;

  // TODO: rest of these ops/mixin for them if can get it to work
  friend bool operator<(const StringIndex& si1, const StringIndex& si2);
  friend bool operator==(const StringIndex& si1, const StringIndex& si2);
  friend bool operator!=(const StringIndex& si1, const StringIndex& si2);
};


// Need to think of simplest way with checks to turn a stringindex into a printed grid
// And then find out how to do the InequalityMixin thing
// And think of nice way to expose this as an interface
// Don't overcomplicate it!
// TODO: move delim default value to header file

std::string stringindex_to_grid_string(
    const Grid& gridp,
    const StringIndex& si,
    const std::string_view spacer_delim = " ")
{
  const auto& grid = *gridp;
  const auto newline = "\n";
  std::string s;

  using std::begin;
  using std::end;
  /* More generic, less readable?.. */
  for (auto i_it = begin(grid); i_it != end(grid); std::advance(i_it, 1))
  {
    bool prepend_delim = false;
    for (auto j_it = begin(*i_it); j_it != end(*i_it); std::advance(j_it, 1))
    {
      if (prepend_delim)
        s.append(spacer_delim);
      prepend_delim = true;
      const auto i = static_cast<std::size_t>(std::distance(begin(grid), i_it));
      const auto j = static_cast<std::size_t>(std::distance(begin(*i_it), j_it));

      // If the index is in the word found, draw the letter in the grid by
      // it back onto s. Otherwise push an empty space.
      if (std::find(si.indexes().begin(), si.indexes().end(), Index{i, j})
          != si.indexes().end())
      {
        s.push_back(grid.at(i).at(j));
      } else
      {
        s.push_back(' ');
      }
    }
    s.append(newline);
  }
  return s;
}

// Sort by length (descending, longest first) then lexicographic)
bool operator<(const StringIndex& si1, const StringIndex& si2)
{
  if (si1.string_.size () != si2.string_.size())
    return si1.string_.size() > si2.string_.size();
  return si1.string_ < si2.string_;
}

bool operator==(const StringIndex& si1, const StringIndex& si2)
{
  return std::tie(si1.string_, si1.indexes_) ==
      std::tie(si2.string_, si2.indexes_);
}

bool operator!=(const StringIndex& si1, const StringIndex& si2)
{
  return !(si1 == si2);
}

// Possible TODO: rewrite with iterators, see if neater
class StringIndexes
{
public:

  // Have again chosen not to use perfect forwarding here for better error
  // message
  void insert(StringIndex si)
  {
    // Unsure of evaluation order guarantees here (with or without c++17 changes).
    // https://en.cppreference.com/w/cpp/language/eval_order - Rules - point 15
    // means I think this would be unspecified so we need an additional line

    // Insert in sorted position if not already there
    const auto pos = std::lower_bound(
          stringindexes_.begin(), stringindexes_.end(), si);
    if (pos == stringindexes_.end() || *pos != si)
      stringindexes_.insert(pos, std::move(si));
  }

  std::vector<std::string> words() const
  {
    std::vector<std::string> words;
    words.reserve(stringindexes_.size());
    for (const auto& p: stringindexes_)
      words.emplace_back(p.string());
    return words;
  }

  // Solution for now to remove words from this
  template<class Pred>
  void filter(Pred&& pred)
  {
    stringindexes_.erase(
          std::remove_if(stringindexes_.begin(), stringindexes_.end(), pred)
          , stringindexes_.end());
  }

  // Returns vector of pairs, where first is the word, and second the grid
  // (multi line)
  std::vector<std::pair<std::string, std::string>> to_grid_strings() const
  {
    std::vector<std::pair<std::string, std::string>> string_index_printable;
    for (const auto& si: stringindexes_)
    {
      string_index_printable.push_back(
      {si.string(), stringindex_to_grid_string(grid_, si)});
    }
    return string_index_printable;
  }

  StringIndexes(Grid grid) : grid_(std::move(grid))
  {}

private:
  Grid grid_;
  std::vector<StringIndex> stringindexes_;
};

/* Several points - for the find_words major loop we don't need to start at
 * dictionary.begin() every time if remembered where we were in dictionary - or
 * even if we stored all the iters from where we were so could go to them when
 * popped back. */
/* For back of the search let's just try a simple index on the next letter as
 * the end point. Then should go and see what the usual distances are. */

std::pair<std::vector<std::string>, std::vector<std::vector<Index>>>
find_words(const Dictionary& dictionary, const Grid& grid, const Index start)
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
    spdlog::debug("\nIteration");
    JR_ASSERT(!a.empty() && !a.back().empty());
    const auto& last = a.back().front();

    spdlog::debug("Tail is {}", tail_word);
    spdlog::debug("At position {} {}", last, index_to_char(last));

    const auto tail_word_it = std::lower_bound(
        dictionary.begin(), dictionary.end(), tail_word);
    if (tail_word_it != dictionary.end() && !(tail_word < *tail_word_it))
    /* if (test_contains(dictionary, tail_word)) */
    {
      spdlog::debug("Outputting: {}", tail_word);
      found_words.push_back(tail_word);
      found_indexes.push_back(tail_indexes);
      JR_ASSERT(tail_word.size() == tail_indexes.size());
    }

    surrounding(last.first, last.second, grid, next_indexes);
    spdlog::debug("Surrouding are {} {}", next_indexes, indexes_to_word(grid, next_indexes));

    /* Remove surrounding indexes that would bite tail_indexes */
    next_indexes.erase(std::remove_if(next_indexes.begin(), next_indexes.end(),
        [&tail_indexes] (const auto& val)
        {
          return std::find(tail_indexes.begin(),
              tail_indexes.end(), val) != tail_indexes.end();
        }), next_indexes.end());
    spdlog::debug("Surrouding are now {}", next_indexes);

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
      spdlog::debug("tail_word is {}", tail_word);
      /* if (!vec_contains_string_that_starts_with(dictionary, tail_word)) */
      if (!vec_contains_string_that_starts_with(tail_word_it, dictionary.end(),
            tail_word))
      {
        spdlog::debug("Rejecting word prefix {}", tail_word);
      } else
      {
        new_layer.emplace_back(i, j);
      }
    }
    /* Remove extra trailing char */
    tail_word.pop_back();

    if (!new_layer.empty())
    {
      spdlog::debug("Adding new layer of letters {}", indexes_to_word(grid,
            new_layer));
      tail_indexes.push_back(new_layer[0]);
      tail_word.push_back(index_to_char(new_layer[0]));
      a.emplace_back(std::move(new_layer));
    } else
    {
      spdlog::debug("Popping a.back().front() as no letters");
      spdlog::debug("Losing {}", last);
      spdlog::debug("a: {}", a);
      while (!a.empty() && a.back().size() <= 1)
      {
        spdlog::debug("Popping additional size 1 {}", a.back());
        a.pop_back();
        tail_indexes.pop_back();
        tail_word.pop_back();
      }
      if (!a.empty())
      {
        spdlog::debug("Popping last of back {}", a.back().front());
        spdlog::debug("tail_indexes: {} vs a: {} vs a.back(): {}", tail_indexes, a, a.back());
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
        spdlog::debug("after ops: tail_indexes: {} vs a: {} vs a.back(): {}", tail_indexes, a, a.back());
      }
      spdlog::debug("a: {}", a);

      /* assert(!tail_indexes.empty()); */
    }
  }

  return {found_words, found_indexes};
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

std::vector<std::string> readlines(const std::filesystem::path& p)
{
  std::vector<std::string> lines;
  std::ifstream f{p};
  JR_ASSERT(f, "Error reading file {}", p);
  /* I hate how primitive assert is. Note to self to find a better solution. */
  /* assert */
  /* spdlog::error(string_view_t fmt, const Args &args...) */
  for (std::string line; std::getline(f, line);)
  {
    lines.emplace_back(std::move(line));
  }
  return lines;
}

/* To get just the list of words flatten the .first returned vector  */
std::pair<
  std::vector<std::vector<std::string>>,
  std::vector<std::vector<std::vector<Index>>>
>
solve_words_indexes(const Dictionary& dict, const Grid& grid)
{
  std::vector<std::vector<std::string>> words_found;
  std::vector<std::vector<std::vector<Index>>> list_of_indexes_found;

  /* More generic, less readable?.. */
  using std::begin;
  using std::end;
  for (auto i_it = begin(*grid); i_it != end(*grid); std::advance(i_it, 1))
  {
    for (auto j_it = begin(*i_it); j_it != end(*i_it); std::advance(j_it, 1))
    {
      auto [words, indexes] = find_words(dict, grid, Index{
          std::distance(begin(*grid), i_it),
          std::distance(begin(*i_it), j_it),
          });
      words_found.push_back(std::move(words));
      list_of_indexes_found.push_back(std::move(indexes));
    }
  }
  return {words_found, list_of_indexes_found};
}

std::vector<std::string> solve(
    const Dictionary& dict, const Grid& grid)
{
  /* list_of_words - std::vector<std::vector<std::string>> */
  auto [list_of_words, _] = solve_words_indexes(dict, grid);
  std::vector<std::string> words_output;
  for (auto& words: list_of_words)
  {
    words_output.insert(words_output.end(),
        std::make_move_iterator(words.begin()),
        std::make_move_iterator(words.end()));
  }
  sort_unique(words_output);
  return words_output;
}

//sortable
//to_string() / operator std::string()

} // namespace wordsearch_solver
