#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include <array>
#include <bitset>
#include <deque>
#include <filesystem>
#include <functional>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <set>
#include <utility>

/* #include "prettyprint.hpp" */
#include "prettyprint.hpp"
#include <fmt/core.h>
#include "spdlog/spdlog.h"
#include <spdlog/common.h>
#include <fmt/format.h>
/* #include <range/v3/view/join.hpp> */
#include <fmt/ostream.h> // Need for printing vector etc. with prettyprint
//Create and return a shared_ptr to a multithreaded console logger.
/* #include "spdlog/sinks/stdout_color_sinks.h" */
/* #include <range/v3/all.hpp> */
#include <boost/container/small_vector.hpp>

#include "magic_enum.hpp"

using namespace std::literals;

template<class T>
std::string join(const T& container, const std::string_view delim)
{
  return fmt::to_string(
      fmt::join(std::begin(container), std::end(container), delim));
  /* std::string result; */
  /* auto it = std::begin(container); */
  /* if (it == std::end(container)) */
  /* { */
  /*   return result; */
  /* } */
  /* fmt::format_to(std::back_inserter(result), "{}", *it); */
  /* std::string fmt_string{}; */
  /* fmt_string += delim; */
  /* fmt_string += "{}"s; */
  /* for (++it; it != std::end(container); ++it) */
  /* { */
  /*   fmt::format_to(std::back_inserter(result), fmt_string, *it); */
  /* } */
  /* return result; */
}

template<class T>
std::string join(T&& container)
{
  return "[" + join(std::forward<T>(container), ", ") + "]";
}

template <class T>
struct InequalityComparableMixin
{
  bool operator!=(const T& t) const noexcept(
      noexcept(static_cast<T*>(std::declval<InequalityComparableMixin*>())->operator==(t)))
  {
    return !static_cast<const T*>(this)->operator==(t);
  }
};

struct Index
{
  using value_type = std::size_t;
  value_type i;
  value_type j;


  constexpr Index(): i{0}, j{0} {}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
  constexpr Index(const value_type i, const value_type j): i{i}, j{j}
  {}
#pragma GCC diagnostic pop

  constexpr bool operator==(const Index& index) const noexcept
  {
    return i == index.i && j == index.j;
  }
  constexpr bool operator!=(const Index& index) const noexcept(noexcept(
        std::declval<Index>() == std::declval<Index>()))
  {
    return !this->operator==(index);
  }
  /* Index(const Index&) = default; */
  /* Index(Index&&) = default; */
  /* Index& operator=(Index&& index) = default; */
  /* Index(Index&&) = default; */
};

std::ostream& operator<<(std::ostream& os, const Index& index)
{
  return os << "[" << index.i << ", " << index.j << "]";
}

namespace std {

  template <>
  struct hash<Index>
  {
    std::size_t operator()(const Index& i) const
    {
      // Can make this much better if compile time values for limits
      return std::hash<Index::value_type>()(i.i) ^ std::hash<Index::value_type>()(i.j);
    }
  };

}

static_assert(std::is_trivially_copyable_v<Index>);

// Not clear on differences here
static_assert(std::is_trivially_move_constructible_v<Index>);
static_assert(std::is_nothrow_move_constructible_v<Index>);

static_assert(noexcept(std::declval<Index>() == std::declval<Index>()));
static_assert(noexcept(std::declval<Index>() != std::declval<Index>()));

template <class T>
std::vector<Index>
old_surrounding(const std::size_t i, const std::size_t j,
    const std::vector<T>& grid)
{
  /* assert(i <= i_size); */
  /* assert(j <= j_size); */

  std::vector<Index> result;
  result.reserve(8);

  /* {i, j + 1}, // E */
  /* {i + 1, j + 1}, // SE */
  /* {i + 1, j} // S */

  /* {i + 1, j - 1} // SW */
  /* {i, j - 1} // W */
  /* {i - 1, j - 1} // NW */

  /* {i - 1, j} // N */
  /* {i - 1, j + 1} // NE */
  result.resize(8);
  const auto insert_if_valid = [&grid, &result] (const auto i, const auto j)
  {
    // Could use limits max type, this works for signed too, for now size_t
    // anyway
    const std::size_t lower_lim = -1UL;
    if (i != lower_lim && j != lower_lim &&
        i < grid.size() && j < grid[i].size())
    {
      result.emplace_back(i, j);
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

  return result;
}

template <class T>
void surrounding(const std::size_t i, const std::size_t j,
    const std::vector<T>& grid, std::vector<Index>& result)
{
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

  result.resize(8);
  auto last_result = result.begin();
  const auto insert_if_valid = [&last_result, &grid]
    (const auto i, const auto j)
  {
    // Could use limits max type, this works for signed too, for now size_t
    // anyway
    const std::size_t lower_lim = -1UL;
    if (i != lower_lim && j != lower_lim &&
        i < grid.size() && j < grid[i].size())
        /* i < grid.size() && j < grid.at(i).size()) */
    {
      last_result->i = i;
      last_result->j = j;
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

template<class C, class V>
  /* typename = std::enable_if_t<std::is_convertible_v<V, C::value_type>>> */
constexpr bool rcontains(const C& container, const V val)
{
  return std::find(container.rbegin(), container.rend(), val) != container.rend();
}

template<class Container, class Value>
constexpr bool contains(const Container& container, const Value value)
{
  return std::find(container.begin(), container.end(), value) != container.end();
}

template<class Container, class Value>
constexpr bool test_contains(const Container& container, const Value value)
{
  /* return std::find(container.begin(), container.end(), value) != container.end(); */
  return std::binary_search(container.begin(), container.end(), value);
  /* return container.find(value) != container.end(); */
}

template <class TwoDArray, class Indexes>
std::string indexes_to_word(const TwoDArray& grid, const Indexes& tail)
{
  std::string word;
  /* This nasty + 1 is here as currently the result of this string is used again
   * but a char is added on (and changed). I cannot see a nicer way of doing
   * this without a performance penalty (*potential* extra allocation to
   * increase size of string's backing buffer by 1) currently.*/
  word.reserve(tail.size() + 1);
  spdlog::debug("Grid: {}", grid);
  for (const auto [i, j]: tail)
  {
    spdlog::debug("Index [{}, {}]", i, j);
    word.push_back(grid.at(i).at(j));
  }
  return word;
}

/* Sadly from looking at a call graph it seems that the std::string_view is not
 * optimised out whereas passing needle by const& is */
template<class Container>
bool vec_contains_string_that_starts_with(
      const Container& haystack,
      const std::string& needle)
      /* const std::string_view needle) */
{
  const auto it = std::lower_bound(haystack.begin(), haystack.end(), needle);
  // Hacked for now for std::set
  // Well this sucks
  // Requires std::set<T, std::less<>>
  /* const auto it = haystack.lower_bound(needle); */
  /* const auto it = haystack.lower_bound(std::string{needle}); */
  /* Avoids using substr on needle - needle was string_view but time being
   * wasted calling operator string on the view for substr, this (hopefully!)
   * avoids any extraneous string */
  if (it == haystack.end()) return false;
  const auto& haystack_string = *it;
  const auto size = static_cast<
    std::decay_t<decltype(needle)>::difference_type>(
      std::min(haystack_string.size(), needle.size()));
  return std::equal(
      haystack_string.begin(), std::next(haystack_string.begin(), size),
      needle.begin(), std::next(needle.begin(), size));
}

template<class Dictionary, class T>
std::pair<std::vector<std::string>, std::vector<std::vector<Index>>>
find_words(
    const Dictionary& dictionary,
    const std::vector<T>& grid,
    const Index start
    )
{
  std::vector<std::string> found_words;
  std::vector<std::vector<Index>> found_indexes;
  std::vector<std::deque<Index>> a{};
#ifndef OLD_SURROUNDING
  std::vector<Index> next_indexes;
#endif

  /* a.push_back({{0, 0}}); */
  a.push_back({start});
  while (!a.empty())
  {
    spdlog::debug("\nIteration");
    assert(!a.empty() && !a.back().empty());
    const auto& last = a.back().front();

    /* Build tail */
    std::vector<Index> tail{};
    tail.reserve(a.size());
    for (const auto& vec: a)
    {
      assert(!vec.empty());
      tail.push_back(vec.front());
    }
    auto tail_word = indexes_to_word(grid, tail);
    spdlog::debug("Tail is {}", tail_word);
    spdlog::debug("At position {} {}", last, grid.at(last.i).at(last.j));

    /* if (std::find(dictionary.begin(), dictionary.end(), tail_word) != dictionary.end()) */
    if (test_contains(dictionary, tail_word))
    {
      spdlog::debug("Outputting: {}", tail_word);
      found_words.push_back(tail_word);
      found_indexes.push_back(tail);
      assert(tail_word.size() == tail.size());
    }

    /* index_to_letter(grid, index); */

#ifdef OLD_SURROUNDING
    auto next_indexes = old_surrounding(last.i, last.j, grid);
#else
    surrounding(last.i, last.j, grid, next_indexes);
#endif
    spdlog::debug("Surrouding are {} {}", next_indexes, indexes_to_word(grid, next_indexes));

    /* Remove surrounding indexes that would bite tail */
    next_indexes.erase(std::remove_if(next_indexes.begin(), next_indexes.end(),
        [&tail] (const auto& val)
        {
          return std::find(tail.begin(), tail.end(), val) != tail.end();
        }), next_indexes.end());
    spdlog::debug("Surrouding are now {}", next_indexes);

    /* Remove surrounding indexes if would not ever form word */
    std::deque<Index> new_layer;
    /* new_layer.reserve(8); */

    /* This last char should be changed every loop iteration */
    tail_word.push_back('\0');
    for (const auto [i, j]: next_indexes)
    {
      const char letter = grid.at(i).at(j);
      auto& new_word = tail_word;
      new_word.back() = letter;
      /* const auto new_word = tail_word + letter; */
      static_assert(std::is_same_v<
          std::remove_reference_t<
          std::remove_cv_t<decltype(new_word)>>, std::string>);
      /* const std::string_view new_word = tail_word + letter; */
      spdlog::debug("new_word is {}", new_word);
      /* if (!first_letter.contains_prefix(new_word)) */
      /* { */
      if (!vec_contains_string_that_starts_with(dictionary, new_word))
      {
        spdlog::debug("Rejecting word prefix {}", new_word);
      } else
      {
        new_layer.emplace_back(i, j);
      }
    }

    if (!new_layer.empty())
    {
      spdlog::debug("Adding new layer of letters {}", indexes_to_word(grid, new_layer));
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
      }
      if (!a.empty())
      {
        spdlog::debug("Popping last of back {}", a.back().front());
        a.back().pop_front();
      }
      spdlog::debug("a: {}", a);
    }
  }

  return {found_words, found_indexes};
}

std::vector<std::string> grid_from_file(
    const std::filesystem::path& wordsearch_file)
{
  namespace fs = std::filesystem;
  fmt::print("Reading wordsearch from file\n");
  /* fs::path wordsearch_file{argv[1]}; */
  assert(fs::exists(wordsearch_file) && "Wordsearch file must exist");
  // Interesting idea - https://stackoverflow.com/a/18514815/8594193
  std::vector<std::string> lines;
  for (auto [inp, line] =
      std::tuple{std::ifstream{wordsearch_file}, std::string{}};
      std::getline(inp, line);)
  {
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
    lines.emplace_back(std::move(line));
    /* fmt::print("Line: {}\n", line); */
  }
  return lines;
}

template<class InputIt, class Size, class UnaryFunction>
InputIt for_each_n(InputIt first, Size n, UnaryFunction f)
{
    for (Size i = 0; i < n; ++first, (void) ++i) {
        f(*first);
    }
    return first;
}

template<class TwoDArray, class Indexes>
const std::vector<std::string> only_indexes_in_grid(TwoDArray grid, const Indexes& indexes)
{
  for (auto i = 0ULL; i < grid.size(); ++i)
  {
    for (auto j = 0ULL; j < grid.at(i).size(); ++j)
    {
      if (!contains(indexes, Index{i, j}))
      {
        grid[i][j] = ' ';
      }
    }
  }
  return grid;
}

template<class Grid>
std::string grid_to_string(const Grid& grid)
{
  std::string s;
  for (const auto& row: grid)
  {
    fmt::format_to(std::back_inserter(s), "{}\n", join(row, " "));
  }
  return s;
}

template <typename T, typename = void>
struct is_sortable : std::false_type {};

template <typename T>
struct is_sortable<T,
  std::void_t<
    decltype(std::begin(std::declval<T>())),
    decltype(std::end(std::declval<T>())),
        decltype(std::sort(std::begin(std::declval<T>()), std::end(std::declval<T>())))
    /* decltype(std::declval<T>().begin()), */
    /* decltype(std::declval<T>().end()) */
    >
  >
  : std::true_type {};

int main (int argc, char** argv)
{
  using namespace magic_enum::ostream_operators;

  /* fmt::print(R"(Calling auto a1 = magic_enum::enum_cast<Num>("a");)""\n"); */
  /* constexpr auto a1 = magic_enum::enum_cast<Num>("a").value(); */
  /* fmt::print("Calling auto a2 = magic_enum::enum_cast<Num>(0);\n"); */
  /* constexpr const auto a2 = magic_enum::enum_cast<Num>(0).value(); */
  /* static_assert(a1 == a2); */
  /* static_assert(asdf<int>(0) == 5); */
  /* fmt::print("--\n"); */

  /* Num n1 = Num::a; */
  /* std::optional<int> o1 = {}; */
  /* /1* std::cout << n1 << "\n"; *1/ */
  /* std::cout << n1 << "\n"; */
  /* std::cout << magic_enum::enum_name(n1) << "\n"; */
  /* fmt::print("{}\n", n1); */
  /* std::cout << magic_enum::enum_name(n1) << "\n"; */
  /* std::cout << o1 << "\n"; */
  /* fmt::print("{}\n", o1); */
  /* fmt::print("{}\n", a1); */

  /* fmt::print(__func__); */
  /* fmt::print("\n"); */
  /* fmt::print(__FUNCTION__); */
  /* fmt::print("\n"); */
  /* fmt::print(__PRETTY_FUNCTION__); */
  /* fmt::print("\n"); */

  /* fmt::print("Enum name: {}\n", magic_enum::enum_name(L::b)); */
  /* fmt::print("Enum name: {}\n", magic_enum::enum_name(L::hi)); */

/*   /1* for (const auto& word: first_letter) *1/ */
/*   /1* { *1/ */
/*   /1*   fmt::print("word: {}\n", word); *1/ */
/*   /1* } *1/ */

  /* const auto check = [] ( */
  /*     const std::string_view substring, */
  /*     const std::string_view haystack, */
  /*     const bool result) */
  /* { */
  /*   /1* const auto output = std::lexicographical_compare(substring.begin(), substring.end(), *1/ */
  /*   /1*     haystack.begin(), haystack.end()); *1/ */
  /*   /1* const auto output = substring < haystack; *1/ */
  /*   const auto output = substring == haystack.substr(0, substring.size()); */
  /*   /1* haystack = haystack.substr(0, substring.size()); *1/ */
  /*   if (output == result) */
  /*   { */
  /*     fmt::print("{} in {}, expected: {}, got: {}\n", substring, haystack, result, output); */
  /*   } */
  /*   else */
  /*   { */
  /*     fmt::print("!! {} in {}, expected: {}, got: {}\n", substring, haystack, result, output); */
  /*   } */
  /*   return output; */
  /* }; */

  /* check("", "bbcb", true); */
  /* check("a", "bbcb", false); */
  /* check("ab", "bbcb", false); */
  /* check("b", "bbcb", true); */
  /* check("bbcb", "bbcb", true); */
  /* check("bbca", "bbcb", false); */
  /* check("bbcc", "bbcb", false); */
  /* check("bbc", "bbcb", true); */
  /* check("bb", "bbcb", true); */
  /* check("b", "bbcb", true); */
  /* check("bbcb", "", false); */
  /* fmt::print("\n"); */

  /* check("bc", "bbcb", false); */

  /* const auto f = [&] (const std::string_view s) */
  /* { */
  /*   const auto it = std::lower_bound(words.begin(), words.end(), s); */
  /*   const auto it2 = std::lower_bound(words.begin(), words.end(), s); */
  /*   fmt::print("{} lower_bound -> {}, upper_bound -> {}\n", s, */
  /*       it != words.end() ? *it : "", it2 != words.end() ? *it2 : ""); */
  /* }; */
  /* f("a"); */
  /* f("ab"); */
  /* f("aba"); */
  /* f("abc"); */
  /* f("abd"); */
  /* f("abf"); */
  /* f("abaa"); */
  /* f("abaaa"); */
  /* f("abaab"); */
  /* f("b"); */
  /* f("be"); */
  /* f("be"); */
  /* f("bf"); */

  spdlog::set_level(spdlog::level::info);

  std::vector<std::string> words =
  /* std::set<std::string, std::less<>> words = */
  {
    /* "a", "ab", "abc", "abe", "abaa", */
    /* "be", */
    /* "ca", */
    /* "abcfedghi", */
    /* "abcfedgh", */
    /* "abcfedi", */
  };

  std::ifstream dict{"/home/justin/cpp/word_trie/gb.txt"};
  for (std::string line; std::getline(dict, line);)
  {
    /* words.emplace_back(std::move(line)); */
    /* words.emplace(line); */
    words.emplace_back(std::move(line));
  }

/*   if constexpr (is_sortable<decltype(words)>{}()) */
/*   { */
/*     fmt::print("Sortable\n"); */
/*   } else */
/*   { */
/*     fmt::print("Unsortable\n"); */
/*   } */
/*   /1* std::sort(words.begin(), words.end()); *1/ */
/*   return 0; */

  /* fmt::print("Words: {}\n", words); */
  fmt::print("Words size: {}\n", words.size());

  const auto grid = [&] ()
  {
    if (argc > 1)
    {
      const auto grid = grid_from_file(argv[1]);
      return grid;
    } else
    {
      const std::vector<std::string> grid =
      {
        {'s', 'b', ' ', 'c', 'a', ' '},
        {'g', 'u', 'f'},
        {'p', 's'},
        {' ', 'e', 'r', 's', ' '},
      };
      return grid;
    }
  }();

  fmt::print("Grid:\n", grid);
  for (const auto& row: grid)
  {
    fmt::print("{}\n", join(row, " "));
  }

  const auto size_then_lexi_sorter = [] (const auto& v1, const auto& v2)
  {
    // Intentionally swapped for v2 and v1 for 2nd item
    // (lexicographical_compare) so that we get longest words first then
    // alphabetically sorted
    return std::forward_as_tuple(v1.size(), v2)
      > std::forward_as_tuple(v2.size(), v1);
  };

  std::vector<std::string> all_outputs;

  using Word = std::string;
  using Indexes = std::vector<Index>;
  std::map<std::size_t,
    std::vector<std::pair<Word, Indexes>>,
    std::greater<>> outputs_by_size;

  for (auto i = 0ULL; i < grid.size(); ++i)
  {
    for (auto j = 0ULL; j < grid.at(i).size(); ++j)
    {
      auto [found_words, found_indexes] = find_words(words, grid, {i, j});
      fmt::print("{} {}\n", found_words, found_indexes);

      /* std::sort(found_words.begin(), found_words.end(), size_then_lexi_sorter); */
      /* fmt::print("Output: {}\n", found_words); */
      /* all_outputs.insert(all_outputs.end(), found_words.begin(), found_words.end()); */

      auto word_it = found_words.begin();
      auto index_it = found_indexes.begin();
      assert(found_words.size() == found_indexes.size());
      for (; word_it != found_words.end(); ++word_it, ++index_it)
      {
        outputs_by_size[word_it->size()].emplace_back(*word_it, *index_it);
      }
    }
  }

  /* std::sort(all_outputs.begin(), all_outputs.end(), size_then_lexi_sorter); */
  /* fmt::print("{}\n", all_outputs); */

  /* std::for_each_n(outputs_by_size.begin(), 5, [] (...) {}); */
  for_each_n(outputs_by_size.begin(), 3, [&grid] (const auto& p)
      {
        const auto size = p.first;
        for (const auto& [word, indexes]: p.second)
        {
          fmt::print("Word: {} of size: {} looks like:\n", word, size);
          /* fmt::print("{}\n", indexes_to_word(grid, indexes)); */
          fmt::print("{}\n", grid_to_string(only_indexes_in_grid(grid, indexes)));
        }
      });

  fmt::print("Done!\n");

}

