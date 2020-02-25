#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

//#include "prettyprint.hpp"
//#include <fmt/format.h>

#include <benchmark/benchmark.h>

#include "wordsearch_solver.h"
#include "dictionary_std_set.h"
#include "dictionary_std_unordered_map.h"
#include "dictionary_std_vector.h"
#include "trie.h"
#include "compact_trie2.hpp"

using namespace std::literals;

std::vector<std::string> readlines(const std::filesystem::path& p)
{
  // fmt::print("Reading from {}\n", p.string());
  std::vector<std::string> lines{};
  assert(std::filesystem::is_regular_file(p));
  std::ifstream f{p};
  assert(f.good() && "File not read correctly");
  for (std::string line; std::getline(f, line);)
  {
    lines.emplace_back(std::move(line));
  }
  return lines;
}

static const std::filesystem::path test_cases_dir = "/home/justin/cpp/wordsearch_solvercp/test/test_cases/";

static const auto dict = readlines(test_cases_dir / "dictionary.txt");
static const auto answers = readlines(test_cases_dir / "long_words/answers.txt");
static const auto grid = wordsearch_solver::grid_from_file(
    test_cases_dir / "long_words/wordsearch.txt");

const std::size_t numb_threads = std::max(
    2U, std::thread::hardware_concurrency()) - 1U;

template<class Dict, class Grid>
void bench_long_words(benchmark::State& state, const Dict& dict,
    const Grid& grid)
{
  for (auto _: state)
  {
    benchmark::DoNotOptimize(wordsearch_solver::solve(dict, grid));
    benchmark::ClobberMemory();
  }
}

#define BENCH_SOLVER(Dict) \
  BENCHMARK_CAPTURE(bench_long_words, Dict, \
      Dict{dict}, grid) \
    ->ReportAggregatesOnly() \
    ->Unit(benchmark::kMillisecond);
    // ->Threads(static_cast<int>(numb_threads))
    // ->Repetitions(static_cast<int>(numb_threads))
    // ->MinTime(1)

BENCH_SOLVER(trie::CompactTrie)
BENCH_SOLVER(CompactTrie2)
BENCH_SOLVER(trie::Trie)
// BENCH_SOLVER(DictionaryStdVector)
// BENCH_SOLVER(DictionaryStdSet)
// BENCH_SOLVER(DictionaryStdUnorderedMap)

#undef BENCH_SOLVER

// template <class ...ExtraArgs>
// void BM_takes_args(benchmark::State& state, ExtraArgs&&... extra_args)
// {


// }
// // Registers a benchmark named "BM_takes_args/int_string_test" that passes
// // the specified values to `extra_args`.
// BENCHMARK_CAPTURE(BM_takes_args, int_string_test, 42, std::string("abc"))
  // ->ThreadPerCpu();

// static void push(benchmark::State& state)
// {

  // // Perform setup here
  // // wordsearch_solver::
  // for (auto _ : state)
  // {
    // // This code gets timed

    // // std::ifstream in(filepath);
    // // std::vector<std::string> lines;
    // // for (std::string line; std::getline(in, line);)
    // // {
      // // lines.push_back(line);
    // // }
  // }
// }
// // Register the function as a benchmark
// BENCHMARK(push)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
