//#include "prettyprint.hpp"

// #include "wordsearch_solver.h"
// #include "dictionary_std_set.h"
// #include "dictionary_std_unordered_map.h"
// #include "dictionary_std_vector.h"
// #include "trie.hpp"
// #include "compact_trie2.hpp"
// #include "compact_trie.hpp"
// #include "utility/utility.hpp"

#include <benchmark/benchmark.h>

#include "wordsearch_solver/wordsearch_solver.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

static const std::filesystem::path test_cases_dir = "../test/test_cases/";

static const auto dict =
    utility::read_file_as_lines(test_cases_dir / "dictionary.txt");
// static const auto grid = solver::make_grid(
// utility::read_file_as_lines(test_cases_dir / "long_words/wordsearch.txt"));
static const auto grid = solver::make_grid(
    utility::read_file_as_lines(test_cases_dir / "massive_wordsearch.txt"));

const std::size_t numb_threads =
    std::max(2U, std::thread::hardware_concurrency()) - 1U;

template <class Dict, class Grid>
void bench_long_words(benchmark::State &state, const Dict &dict,
                      const Grid &grid) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(solver::solve(dict, grid));
    benchmark::ClobberMemory();
  }
}

#define BENCH_SOLVER(Dict)                                                     \
  BENCHMARK_CAPTURE(bench_long_words, Dict, Dict{dict}, grid)                  \
      ->ReportAggregatesOnly()                                                 \
      ->Unit(benchmark::kMillisecond);
// ->Threads(static_cast<int>(numb_threads))
// ->Repetitions(static_cast<int>(numb_threads))
// ->MinTime(1)

// BENCH_SOLVER(compact_trie::CompactTrie)
// BENCH_SOLVER(compact_trie2::CompactTrie2)
#ifdef WORDSEARCH_SOLVER_HAS_trie
BENCH_SOLVER(trie::Trie)
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie
BENCH_SOLVER(compact_trie::CompactTrie)
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie2
BENCH_SOLVER(compact_trie2::CompactTrie2)
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_vector
BENCH_SOLVER(dictionary_std_vector::DictionaryStdVector)
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_set
BENCH_SOLVER(dictionary_std_set::DictionaryStdSet)
#endif

// BENCH_SOLVER(DictionaryStdVector)
// BENCH_SOLVER(DictionaryStdSet)
// BENCH_SOLVER(DictionaryStdUnorderedMap)

#undef BENCH_SOLVER

// template <class... ExtraArgs>
// void BM_takes_args(benchmark::State &state, ExtraArgs &&... extra_args) {}
// // Registers a benchmark named "BM_takes_args/int_string_test" that passes
// // the specified values to `extra_args`.
// BENCHMARK_CAPTURE(BM_takes_args, int_string_test, 42, std::string("abc"))
// ->ThreadPerCpu();

// static void push(benchmark::State &state) {

// // Perform setup here
// // wordsearch_solver::
// for (auto _ : state) {
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

BENCHMARK_MAIN();
