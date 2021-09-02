#include <benchmark/benchmark.h>

#include "wordsearch_solver/wordsearch_solver.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using namespace std::literals;

// Currently this uses macros and bypasses the solver::SolverFactory creation
// for the dicts, therefore it doesn't measure the std::variant overhead too.
// Converting this to use our own main() function and
// benchmark::RegisterBenchmark is possible, but requires the underlying types
// to be copy constructible, which currently some of the tries aren't.

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
void bench_long_words(benchmark::State& state, const Dict& dict,
                      const Grid& grid) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(solver::solve(dict, grid));
    benchmark::ClobberMemory();
  }
}

void bench_solver_init(benchmark::State& state,
                       const std::string_view dict_solver) {
  solver::SolverDictFactory solvers{};
  for (auto _ : state) {
    benchmark::DoNotOptimize(solvers.make(dict_solver, dict));
    benchmark::ClobberMemory();
  }
}

#define BENCH_SOLVER_INIT(DictSolverArg)                                       \
  BENCHMARK_CAPTURE(bench_solver_init, DictSolverArg, #DictSolverArg)          \
      ->Unit(benchmark::kMillisecond);
// ->ReportAggregatesOnly()

#define BENCH_SOLVER(Dict)                                                     \
  BENCHMARK_CAPTURE(bench_long_words, Dict, Dict{dict}, grid)                  \
      ->Unit(benchmark::kMillisecond);
// ->ReportAggregatesOnly()
// ->Threads(static_cast<int>(numb_threads))
// ->Repetitions(static_cast<int>(numb_threads))
// ->MinTime(1)

// BENCH_SOLVER(compact_trie::CompactTrie)
// BENCH_SOLVER(compact_trie2::CompactTrie2)

// NOTE: I hate macros.
// For this to be "proper" should probably have some way to get the string name
// of each solver from the macros so can do all this programatically. For now,
// this works

#ifdef WORDSEARCH_SOLVER_HAS_trie
BENCH_SOLVER_INIT(trie)
BENCH_SOLVER(trie::Trie)
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie
BENCH_SOLVER_INIT(compact_trie)
BENCH_SOLVER(compact_trie::CompactTrie)
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie2
BENCH_SOLVER_INIT(compact_trie2)
BENCH_SOLVER(compact_trie2::CompactTrie2)
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_vector
BENCH_SOLVER_INIT(dictionary_std_vector)
BENCH_SOLVER(dictionary_std_vector::DictionaryStdVector)
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_set
BENCH_SOLVER_INIT(dictionary_std_set)
BENCH_SOLVER(dictionary_std_set::DictionaryStdSet)
#endif

#undef BENCH_SOLVER
#undef BENCH_SOLVER_INIT

BENCHMARK_MAIN();
