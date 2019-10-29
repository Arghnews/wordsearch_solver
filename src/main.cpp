#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

//#include "prettyprint.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>
#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#include "jr_assert.h"


using namespace std::literals;

#include "lyra/lyra.hpp"
#include "wordsearch_solver.h"

template <>
struct fmt::formatter<wordsearch_solver::StringIndex>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const wordsearch_solver::StringIndex &d, FormatContext &ctx)
  {
    return format_to(ctx.out(), "{} {}", d.string(), d.indexes());
//    return format_to(ctx.out(), "{}", 3);
  }
};

auto now()
{
  return std::chrono::high_resolution_clock::now();
}

// Old comment
// https://en.cppreference.com/w/cpp/numeric/ratio/ratio
// eg. Ratio == std::milli, std::micro,
// Defaults to Ratio of 1/1 ie. seconds as they are not in std::ratio
// Bleh maybe just nicer to do the duration_cast
template <class ToDuration, class Duration>
auto diff(const Duration& from, const Duration& to)
{
  return std::chrono::duration_cast<ToDuration>(to - from);
//  return std::chrono::duration<double, Ratio>(to - from);
}

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
    ;

  auto result = cli.parse({argc, argv});
  if (!result)
  {
//    puts("Error in parsing");
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

  std::vector<std::string> timings;
  auto add_timing = [&timings] (auto&&... args)
  {
    timings.push_back(fmt::format(args...));
  };

  auto t1 = now();
  const auto dict = wordsearch_solver::readlines(args.dictionary_path);
  const auto wordsearch = wordsearch_solver::grid_from_file(
      args.wordsearch_path);
  add_timing("Read inputs {}\n", diff<std::chrono::milliseconds>(t1, now()));


  t1 = now();
  const auto a1 = wordsearch_solver::solve(dict, wordsearch);
  auto answers = a1.words();
  add_timing("Solve {}\n", diff<std::chrono::milliseconds>(t1, now()));

  t1 = now();
  std::unique(answers.begin(), answers.end());
  JR_ASSERT(std::is_sorted(a1.begin(), a1.end()));
  add_timing("Uniqify/assert sorted {}\n", diff<std::chrono::milliseconds>(t1, now()));

  if (args.min_word_len)
  {
    answers.erase(std::remove_if(answers.begin(), answers.end(),
        [&min_word_len = *args.min_word_len] (const auto& word)
        {
          return word.size() < min_word_len;
        }), answers.end());
  }

  t1 = now();
  for (const auto& a: answers)
  {
    fmt::print("a: {}\n", a);
  }
  fmt::print("Answers size: {} {}\n", answers.size(),
             fmt::join(answers.begin(), answers.end(), "\n"));
  add_timing("Print answers {}\n", diff<std::chrono::milliseconds>(t1, now()));

  fmt::print("{}\n", fmt::join(timings.begin(), timings.end(), ""));

}
