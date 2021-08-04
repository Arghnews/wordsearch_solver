//#include "prettyprint.hpp"

// #include "wordsearch_solver.h"
// #include "dictionary_std_set.h"
// #include "dictionary_std_unordered_map.h"
// #include "dictionary_std_vector.h"
// #include "trie.hpp"
// #include "compact_trie2.hpp"
// #include "compact_trie.hpp"
// #include "utility/utility.hpp"

#include "wordsearch_solver/wordsearch_solver.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <args-parser/all.hpp>

#include <gperftools/profiler.h>

using namespace std::literals;

// static const std::filesystem::path test_cases_dir = "../test/test_cases/";

// static const auto dict =
// utility::read_file_as_lines(test_cases_dir / "dictionary.txt");
// static const auto grid = solver::make_grid(
// utility::read_file_as_lines(test_cases_dir /
// "long_words/wordsearch.txt"));
// static const auto grid =
// solver::make_grid(utility::read_file_as_lines("massive_wordsearch.txt"));

// struct SolverQueries {

// };

template <class Dict>
std::size_t main_solve(const Dict &dict, const solver::WordsearchGrid &grid) {
  std::puts("Solving");
  ProfilerRestartDisabled();
  ProfilerEnable();
  const auto result = solver::solve(dict, grid);
  return result.size();
}

// This sucks. Way too much manual crap and no DRY...
// Possible improvements.
// - That magic_get c++17 compile time thing used in an enum library trick
// would probably work.
// - Overhaul the solvers file to have a class that has all this information
// as this is getting too much.
// - Wait for 2050 when c++ has built in proper metaprogramming/static
// reflection.

int main(int argc, char **argv) {
  const std::string help = "./this -d dictionary_path -w wordsearch_path";
  std::string dict_path;
  std::string wordsearch_path;
  std::string solver;

  try {
    Args::CmdLine cmd(argc, argv);

    cmd.addArgWithNameOnly("dict", true, true);
    cmd.addArgWithNameOnly("wordsearch", true, true);
    // Can't figure out how to do groups with this garbage argparser library
    cmd.addArgWithNameOnly("solver", true, true);
    cmd.addHelp(true, argv[0],
                "Usage: ./this --dict path/to/dict --wordsearch path/to/ws");

    cmd.parse(argc, argv);
    dict_path = cmd.value("--dict");
    wordsearch_path = cmd.value("--wordsearch");
    solver = cmd.value("--solver");

  } catch (const Args::HelpHasBeenPrintedException &) {
    return 0;
  } catch (const Args::BaseException &x) {
    Args::outStream() << x.desc() << "\n";

    return 1;
  }

  const auto dict_lines = utility::read_file_as_lines(dict_path);
  const auto grid =
      solver::make_grid(utility::read_file_as_lines(wordsearch_path));

  std::vector<std::string> solvers;
#ifdef WORDSEARCH_SOLVER_HAS_trie
  if (solver == solvers.emplace_back("trie")) {
    main_solve(trie::Trie{dict_lines}, grid);
  }
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie
  else if (solver == solvers.emplace_back("compact_trie")) {
    main_solve(compact_trie::CompactTrie{dict_lines}, grid);
  }
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie2
  else if (solver == solvers.emplace_back("compact_trie2")) {
    main_solve(compact_trie2::CompactTrie2{dict_lines}, grid);
  }
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_vector
  else if (solver == solvers.emplace_back("dictionary_std_vector")) {
    main_solve(dictionary_std_vector::DictionaryStdVector{dict_lines}, grid);
  }
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_set
  else if (solver == solvers.emplace_back("dictionary_std_set")) {
    main_solve(dictionary_std_set::DictionaryStdSet{dict_lines}, grid);
  }
#endif
  else {
    throw std::runtime_error(
        fmt::format("Invalid solver, choose from: {}", solvers));
  }
}
