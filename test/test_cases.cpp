#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

#include <catch2/catch.hpp>
#include "wordsearch_solver.h"

#include "dictionary.h"
#include "dictionary_std_set.h"
#include "trie.h"

using namespace std::literals;

namespace
{

std::vector<std::string> readlines(const std::filesystem::path& p)
{
  std::vector<std::string> lines;
  std::ifstream f{p};
  INFO("File name is " << p);
  REQUIRE(std::filesystem::is_regular_file(p));
  REQUIRE(f.good());
  for (std::string line; std::getline(f, line);)
  {
    lines.emplace_back(std::move(line));
  }
  return lines;
}

template<class Vector>
// Vector sort_unique(Vector&& v)
Vector sort_unique(Vector v)
{
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
  // https://stackoverflow.com/a/12949161/8594193
  // Hurts my head somewhat but I believe is correct
  return v;
}

}

namespace fs = std::filesystem;

const fs::path test_cases_dirname{"test_cases"};
const fs::path dictionary_filename{"dictionary.txt"};
const fs::path answers_filename{"answers.txt"};
const fs::path wordsearch_filename{"wordsearch.txt"};

TEST_CASE( "Check test inputs", "[input]")
{
  const fs::path test_cases_dir(test_cases_dirname);
  REQUIRE(fs::is_directory(test_cases_dir));

  const auto dictionary_path = test_cases_dir / dictionary_filename;
  REQUIRE(fs::is_regular_file(dictionary_path));

  const auto dict_words = sort_unique(::readlines(dictionary_path));

  for (const auto &test_dir : fs::directory_iterator(test_cases_dir))
  {
    if (!fs::is_directory(test_dir))
    {
      continue;
    }
    REQUIRE(fs::is_regular_file(test_dir / wordsearch_filename));

    REQUIRE(fs::is_regular_file(test_dir / answers_filename));
    const auto answers = sort_unique(::readlines(test_dir / answers_filename));
    for (const auto& answer: answers)
    {
      // Waiting for utf8 heaven one day...
      for (const auto c: answer)
      {
        REQUIRE(std::islower(c));
      }
    }

    // std::cout << "Checking " << test_cases_dir / dictionary_name << " vs "
      // << test_dir / answers_name << "\n";
    // std::cout << "Checking " << fs::absolute(test_cases_dir / dictionary_name) << " vs "
      // << fs::absolute(test_dir / answers_name) << "\n";
    INFO("File name is " << test_dir);
//  std::includes requires sorted ranges
    REQUIRE(std::includes(dict_words.begin(), dict_words.end(), answers.begin(),
          answers.end()));
  }
}

TEMPLATE_TEST_CASE( "Test wordsearch/dict implementations", "[test]",
    DictionaryStdSet, Trie)
{
  const auto dictionary_path = test_cases_dirname / dictionary_filename;
  const auto dict_words = sort_unique(::readlines(dictionary_path));

  const wordsearch_solver::Dictionary dict = TestType{dict_words};
  for (const auto &test_dir : fs::directory_iterator(test_cases_dirname))
  {
    if (!fs::is_directory(test_dir))
    {
      continue;
    }
    const auto answers = sort_unique(::readlines(test_dir / answers_filename));
    INFO("File name is " << test_dir << "\n");
    const auto grid = wordsearch_solver::grid_from_file(
        test_dir / wordsearch_filename);
    const auto stringindexes = wordsearch_solver::solve(dict, grid);
    const auto results = sort_unique(stringindexes.words());
    CHECK(std::includes(results.begin(), results.end(),
          answers.begin(), answers.end()));
  }

}
