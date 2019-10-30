#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
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

struct FromTo
{
  using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
  TimePoint from;
  std::optional<TimePoint> to = std::nullopt;
};

// TODO:
// cleaner optional
// polymorphic Times storage for different std::ratio ie. std::chrono::millis
// 	etc. find out how
struct Timekeeper
{

  void start(std::string name)
  {
    JR_ASSERT(!current_timed_name_, "Currently may only time one "
              "thing at once, cannot start timing {} while timing {}",
              name, *current_timed_name_);
    JR_ASSERT(name_indexes_.find(name) == name_indexes_.end(),
             "May not have duplicate key {} in Timekeeper", name);

    if (!insertion_order_.empty())
    {
      longest_name_ = std::max(name.size(), insertion_order_.back()->first.size());
    } else
    {
      longest_name_ = name.size();
    }
    current_timed_name_ = name;
    const auto [it, _] = name_indexes_.emplace(std::move(name), FromTo{now()});
    insertion_order_.push_back(it);
  }

  void stop()
  {
    JR_ASSERT(current_timed_name_, "Cannot call stop while not timing");
    name_indexes_.at(*current_timed_name_).to = now();
    current_timed_name_ = std::nullopt;
  }

  // Can do without the optional but messier
  // std::map iterators valid even after inserts
  // https://stackoverflow.com/questions/6438086/iterator-invalidation-rules

  using KeyTimeMap = std::map<std::string, FromTo>;
  KeyTimeMap name_indexes_;
  std::optional<std::string> current_timed_name_;
  std::vector<KeyTimeMap::const_iterator> insertion_order_;
  std::size_t longest_name_;

  Timekeeper() = default;
  ~Timekeeper()
  {
    JR_ASSERT(!current_timed_name_, "Timekeeper destroyed while still "
                                    "timing {}", *current_timed_name_);
    for (const auto& [name, fromto]: name_indexes_)
    {
      JR_ASSERT(fromto.to, "Never stopped timing {}", name);
      // TODO: change to warning
      JR_ASSERT(fromto.to >= fromto.from, "Recorded negative amount of time. "
                "Careful with benchmark results for {}", name);
    }
  }

  std::string summary() const
  {
    std::string out;
//    fmt::format_to(format_string, "{{{}}}: {{}}\n", longest_name_);
    std::chrono::duration<double> total;
    const auto format_string = fmt::format("{{:<{}}}: {{}}\n", longest_name_);
    for (const auto it: insertion_order_)
    {
      const auto& [name, fromto] = *it;
      JR_ASSERT(fromto.to, "Never stopped timing {}", name);
      total += (*fromto.to - fromto.from);
      auto d = std::chrono::duration_cast<std::chrono::milliseconds>
          (*fromto.to - fromto.from);
      fmt::format_to(std::back_inserter(out), format_string, name, d);
    }
    fmt::format_to(std::back_inserter(out), format_string, "Total (ms)",
          std::chrono::duration_cast<std::chrono::milliseconds>(total));
    fmt::format_to(std::back_inserter(out), format_string, "Total (s)", total);
    return out;
  }
};

Timekeeper& timer()
{
  static Timekeeper timekeeper;
  return timekeeper;
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

//  std::vector<std::string> timings;
//  auto add_timing = [&timings] (auto&&... args)
//  {
//    timings.push_back(fmt::format(args...));
//  };

  timer().start("Read inputs");
  const auto dict = wordsearch_solver::readlines(args.dictionary_path);
  const auto wordsearch = wordsearch_solver::grid_from_file(
      args.wordsearch_path);
  timer().stop();

  timer().start("Solve");
  auto a1 = wordsearch_solver::solve(dict, wordsearch);
  timer().stop();

  timer().start("Sort");
  a1.sort();
  timer().stop();

  timer().start("Unique");
  a1.unique();
  timer().stop();

  auto answers = a1.words();

  timer().start("Uniqify words only/assert sorted");
  std::unique(answers.begin(), answers.end());
  JR_ASSERT(std::is_sorted(a1.begin(), a1.end()));
  timer().stop();

  if (args.min_word_len)
  {
    answers.erase(std::remove_if(answers.begin(), answers.end(),
        [&min_word_len = *args.min_word_len] (const auto& word)
        {
          return word.size() < min_word_len;
        }), answers.end());
  }

  timer().start("Print answers");
  for (const auto& a: answers)
  {
    fmt::print("a: {}\n", a);
  }
  fmt::print("Answers size: {} {}\n", answers.size(),
             fmt::join(answers.begin(), answers.end(), "\n"));
  timer().stop();

  fmt::print("{}\n", timer().summary());

}
