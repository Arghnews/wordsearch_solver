#include "test_cases.hpp"

#include "wordsearch_solver/wordsearch_solver.hpp"

#include <catch2/catch.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/zip.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// TODO:
// Currently test fails because of unsorted thing.
// Could try to create some kind of CRTP wrapper that classes implement/inherit from to implement the contains/further stuff.
// So they all get the constructor wrappers for free.
// Also find cmake way to run tests for one.
// Find cmake restructure to have all targets in top level cmakelists and pass that to solver with variable (easy)
// Fork prettyprint.hpp repo and add nice stuff for optional configure, and cmake support
// Nicer way to pass all classes to run through from build to program would be awesome really.
// Need to copy SFINAE for string_view -> string construction from set to vector

// To make adding a dictionary implementation less painful, add it to this
// "list", delimited by a comma
// #define WORDSEARCH_DICTIONARY_CLASSES compact_trie::CompactTrie, trie::Trie,
// \
  // compact_trie2::CompactTrie2, dictionary_std_vector::DictionaryStdVector, \
  // dictionary_std_set::DictionaryStdSet

using namespace std::literals;

TEMPLATE_TEST_CASE("Test wordsearch/dict implementations", "[test]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  check_inputs();

  const auto dictionary_path = test_cases_dirname / dictionary_filename;
  const auto dict_words = sort_unique(utility::read_file_as_lines(dictionary_path));

  // using Dictionary = wordsearch_solver::Solver<
    // DictionaryStdSet, DictionaryStdVector, trie::CompactTrie>;
  const TestType dict{dict_words};
  for (const auto &test_dir : fs::directory_iterator(test_cases_dirname))
  {
    if (!fs::is_directory(test_dir))
    {
      continue;
    }
    const auto answers = sort_unique(utility::read_file_as_lines(test_dir / answers_filename));
    // INFO("File name is " << test_dir << "\n");
    const auto lines = utility::read_file_as_lines(test_dir / wordsearch_filename);
    // need to make grid
    // solver::Tail
    const auto grid = solver::make_grid(lines);
    const auto word_to_list_of_indexes = solver::solve(dict, grid);
// TODO: fix this
    const auto results = ranges::views::keys(word_to_list_of_indexes)
      | ranges::to<std::vector<std::string>>()
      | ranges::actions::sort | ranges::actions::unique;
    // words | ranges::actions::sort | ranges::actions::unique;
    // ranges::views::keys();
    // ranges::views::keys
    // const auto results = sort_unique(stringindexes.words());
    const auto contains = [] (const auto& haystack, const auto& needle)
    {
      return std::binary_search(haystack.begin(), haystack.end(), needle);
    };
    const auto print_iter = [](const auto& container, const auto it)
    {
      if (it != container.end())
      {
        return *it;
      }
      return std::string{"[end_iterator]"};
    };

    for (const auto& answer: answers)
    {
      const auto present = contains(results, answer);
      if (!present)
      {
        auto first = std::lower_bound(results.begin(), results.end(), answer);
        auto second = first != results.end() ? std::next(first) : results.end();
        if (first != results.begin() && *first > answer)
        {
          --first;
          --second;
        }
        FAIL_CHECK("Answer: \"" << answer << "\" missing from results. "
            "Values at positions that answer should be - before: \""
            << print_iter(results, first) << "\", and after: \""
            << print_iter(results, second) << "\"");
      }
    }
    CHECK(std::includes(results.begin(), results.end(),
          answers.begin(), answers.end()));

  }
}

template <class Range> auto make_adjacent_view(Range &&rng) {
  return ranges::views::zip(ranges::views::all(std::forward<Range>(rng)),
                            ranges::views::all(std::forward<Range>(rng)) |
                                ranges::views::drop(1));
}

inline constexpr const auto comparator_by_size = [](const auto &word0,
                                                    const auto &word1) {
  return std::less<void>{}(word0.size(), word1.size());
};

using T = std::set<std::vector<solver::Index>>;
using MultimapBySize =
    std::multimap<std::string, T, decltype(comparator_by_size)>;

template <class SolverDict>
MultimapBySize solve_to_multimap(const solver::WordsearchGrid &wordsearch_grid,
                                 const SolverDict &solver) {

  const solver::WordToListOfListsOfIndexes results =
      solver::solve(solver, wordsearch_grid);

  MultimapBySize word_to_list_of_indexes{comparator_by_size};

  for (const auto &[k, list_of_list_of_indexes] : results) {

    T set_of_lists_of_indexes;
    set_of_lists_of_indexes.insert(list_of_list_of_indexes.begin(),
                                   list_of_list_of_indexes.end());
    word_to_list_of_indexes.emplace(k, std::move(set_of_lists_of_indexes));
  }

  for (const auto &[word, set_of_lists_of_indexes] : word_to_list_of_indexes) {
    for (const auto &indexes : set_of_lists_of_indexes) {

      for (const auto &i0 : indexes) {
        // Ensure indexes are in bounds
        REQUIRE(i0.y >= 0);
        REQUIRE(i0.y < wordsearch_grid.rows());
        REQUIRE(i0.x >= 0);
        REQUIRE(i0.x < wordsearch_grid.columns());
      }
      REQUIRE(word.size() == indexes.size());
      for (const auto [c, i] : ranges::views::zip(word, indexes)) {
        // Sanity check, ensure the letter at the index is the letter in the
        // word
        REQUIRE(c == wordsearch_grid(i));
      }

      for (const auto [i0, i1] : make_adjacent_view(indexes)) {
        // Ensure indices are adjacent in the wordsearch
        REQUIRE(std::abs(static_cast<long>(i0.y - i1.y)) <= 1);
        REQUIRE(std::abs(static_cast<long>(i0.x - i1.x)) <= 1);
      }
    }
  }
  return word_to_list_of_indexes;
}

template <std::size_t I, class Answer, class... Args>
void check_output_equal_impl(const solver::WordsearchGrid &wordsearch_grid,
                             const Answer &answer,
                             const std::tuple<Args...> &args) {
  if constexpr (I >= sizeof...(Args)) {
    return;
  } else {
    REQUIRE(solve_to_multimap(wordsearch_grid, std::get<I>(args)) == answer);
    check_output_equal_impl<I + 1, Answer, Args...>(wordsearch_grid, answer,
                                                    args);
  }
}

template <class... Args>
void check_output_equal(const solver::WordsearchGrid &wordsearch_grid,
                        const std::vector<std::string> &words) {
  if constexpr (sizeof...(Args) > 1) {
    const auto dicts = std::tuple{Args{words}...};
    const auto answer = solve_to_multimap(wordsearch_grid, std::get<0>(dicts));
    check_output_equal_impl<1>(wordsearch_grid, answer, dicts);
  }
}

TEST_CASE("Test all dictionary implementation's output equal each other",
          "[test]") {
  check_inputs();

  for (const auto &test_dir : fs::directory_iterator(test_cases_dirname)) {
    if (!fs::is_directory(test_dir)) {
      continue;
    }
    const auto lines =
        utility::read_file_as_lines(test_dir / wordsearch_filename);
    const auto grid = solver::make_grid(lines);

    check_output_equal<WORDSEARCH_DICTIONARY_CLASSES>(grid, lines);
  }
}
