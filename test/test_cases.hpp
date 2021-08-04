#ifndef TEST_CASES_HPP
#define TEST_CASES_HPP

#include "wordsearch_solver/utility/utility.hpp"

#include "range/v3/algorithm/all_of.hpp"
#include <catch2/catch.hpp>
#include <fmt/format.h>

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;
inline const fs::path test_cases_dirname{"test_cases"};
inline const fs::path dictionary_filename{"dictionary.txt"};
inline const fs::path answers_filename{"answers.txt"};
inline const fs::path wordsearch_filename{"wordsearch.txt"};

template <class Vector>
// Vector sort_unique(Vector&& v)
inline Vector sort_unique(Vector v) {
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
  // https://stackoverflow.com/a/12949161/8594193
  // Hurts my head somewhat but I believe is correct
  return v;
}

// TODO: move this into setup for template fixture
inline void check_inputs() {
  const fs::path test_cases_dir(test_cases_dirname);
  INFO(fmt::format("Looking for test_cases_dir: {}", test_cases_dir.string()));
  // REQUIRE(fs::is_directory(test_cases_dir));

  const auto dictionary_path = test_cases_dir / dictionary_filename;
  INFO(fmt::format("Looking for dictionary filepath: {}",
                   dictionary_path.string()));
  // REQUIRE(fs::is_regular_file(dictionary_path));

  const auto dict_words =
      sort_unique(utility::read_file_as_lines(dictionary_path));

  // TODO:
  // replace the filesystem calls in here with ones from
  // https://emscripten.org/docs/api_reference/Filesystem-API.html

  for (const auto &test_dir : fs::directory_iterator(test_cases_dir)) {
    if (!fs::is_directory(test_dir)) {
      std::cout << "Skipperino " << test_dir.path().string() << "\n";
      continue;
    }
    // REQUIRE(fs::is_regular_file(test_dir / wordsearch_filename));

    // REQUIRE(fs::is_regular_file(test_dir / answers_filename));
    const auto answers =
        sort_unique(utility::read_file_as_lines(test_dir / answers_filename));
    for (const auto &answer : answers) {
      // Waiting for utf8 heaven one day... UD on non-ascii
      INFO(fmt::format("Answer word has non lowercase: {}", answer));
      REQUIRE(
          ranges::all_of(answer, [](const auto c) { return std::islower(c); }));
    }

    // std::cout << "Checking " << test_cases_dir / dictionary_name << " vs "
    // << test_dir / answers_name << "\n";
    // std::cout << "Checking " <<
    //    fs::absolute(test_cases_dir / dictionary_name) << " vs "
    // << fs::absolute(test_dir / answers_name) << "\n";
    // Commented out for now, until can figure out how to print this with libc++
    // INFO("File name is " << test_dir);
    // std::includes requires sorted ranges
    REQUIRE(std::includes(dict_words.begin(), dict_words.end(), answers.begin(),
                          answers.end()));
  }
}

#endif // TEST_CASES_HPP
