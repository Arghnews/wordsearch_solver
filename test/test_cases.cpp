#include <algorithm>
#include <filesystem>
#include <optional>
#include <vector>

#include <catch2/catch.hpp>
#include "wordsearch_solver.h"

using namespace std::literals;

TEST_CASE( "Test indexes line up", "[indexes]")
{

  using namespace wordsearch_solver;
  namespace fs = std::filesystem;

  // const fs::path test_cases_path("test_cases");
  const fs::path test_cases_path("test_cases");
  REQUIRE(fs::exists(test_cases_path));

  const fs::path dictionary_name = "dictionary.txt";
  const auto default_dictionary_path = test_cases_path / dictionary_name;

  // TODO: possibly implement overriding dictionary in subdirs
  const auto default_dictionary = fs::exists(default_dictionary_path)
    ? std::optional{readlines(default_dictionary_path)} : std::nullopt;

  for (auto& test_dir: fs::directory_iterator(test_cases_path))
  {
    if (!fs::is_directory(test_dir))
    {
      continue;
    }
    const auto wordsearch = grid_from_file(test_dir / "wordsearch.txt");
    auto answers = readlines(test_dir / "answers.txt");
    for (const auto& answer: answers)
    {
      // Waiting for utf8 heaven one day...
      for (const auto c: answer)
      {
        REQUIRE(std::islower(c));
      }
    }

    INFO("File name is " << test_dir);
    REQUIRE(default_dictionary.has_value());
    // REQUIRE(
        // (default_dictionary || fs::exists(test_dir / dictionary_name)));
    auto words = solve(default_dictionary.value(), wordsearch).words();
    std::sort(words.begin(), words.end());
//    CAPTURE(words);
    REQUIRE(std::includes(words.begin(), words.end(), answers.begin(),
          answers.end()));
  }

}

