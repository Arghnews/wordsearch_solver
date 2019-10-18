#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

//#include "prettyprint.hpp"
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#include <fmt/ostream.h>
#include "jr_assert.h"

using namespace std::literals;

#include "lyra/lyra.hpp"
#include "wordsearch_solver.h"

int main(int argc, char** argv)
{

  struct
  {
    std::string dictionary_path;
    std::string wordsearch_path;
    bool show_help = false;
    spdlog::level::level_enum level = spdlog::level::info;
    std::optional<std::size_t> min_word_len;
    bool sort_by_length = false;
  } args;

  auto cli = lyra::cli_parser()
    |
    lyra::help(args.show_help)
    |
    lyra::opt(args.dictionary_path, "Dictionary file")
    ["-d"]["--dictionary"]
    ("Dictionary or wordlist file").required()
    |
    lyra::opt(args.wordsearch_path, "Wordsearch file")
    ["-w"]["--wordsearch"]
    ("Wordsearch file").required()
    |
    lyra::opt([&] (bool) { args.level = spdlog::level::debug; })
    ["-v"]["--verbose"] ("Verbose flag")
    |
    lyra::opt(args.min_word_len, "Length")
    ["-m"]["--min-length"] ("Only print words longer than this")
    |
    lyra::opt(args.sort_by_length)
    ["-l"]["--sort-by-length"] ("Sort by length, then alphabetically")
    ;

  auto result = cli.parse({argc, argv});
  if (!result)
  {
    fmt::print("Error in parsing command line arguments: {}\n",
        result.errorMessage());
    return 1;
  }

  if (args.show_help)
  {
    fmt::print("{}\n", cli);
    return 0;
  }

  spdlog::set_level(args.level);

  // Yes yes technically race condition. But nice error messages are nice.
  JR_ASSERT(std::filesystem::exists(args.dictionary_path),
      "Dictionary file does not seem to exist at {}", args.dictionary_path);
  JR_ASSERT(std::filesystem::exists(args.wordsearch_path),
      "Wordsearch file does not seem to exist at {}", args.wordsearch_path);

  const auto dict = wordsearch_solver::readlines(args.dictionary_path);
  const auto wordsearch = wordsearch_solver::grid_from_file(
      args.wordsearch_path);

  auto answers = wordsearch_solver::solve(dict, wordsearch);

  if (args.min_word_len)
  {
    answers.erase(std::remove_if(answers.begin(), answers.end(),
        [&min_word_len = *args.min_word_len] (const auto& word)
        {
          return word.size() < min_word_len;
        }), answers.end());
  }

  if (args.sort_by_length)
  {
    // Sort by size in descending order then by lexicographical
    std::sort(answers.begin(), answers.end(),
        [] (const auto& w1, const auto& w2)
        {
          if (w1.size() != w2.size())
          {
            return w1.size() > w2.size();
          }
          return w1 < w2;
        });
  }

  fmt::print("{}\n", fmt::join(answers.begin(), answers.end(), "\n"));

}
