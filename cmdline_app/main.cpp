#include "wordsearch_solver/wordsearch_solver.hpp"

#include <args-parser/all.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gperftools/profiler.h>
#include <range/v3/view/all.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

class SolverDictWrapper {
  std::variant<WORDSEARCH_DICTIONARY_CLASSES> t_;

  template <class Func> auto run(Func &&func) const {
    return std::visit(std::forward<Func>(func), t_);
  }

public:
  template <class SolverDict, class Words>
  SolverDictWrapper(const SolverDict &solver_dict, Words &&words)
      : t_(solver_dict, std::forward<Words>(words)) {}

  std::size_t size() const {
    return this->run([](const auto &t) { return t.size(); });
  }

  bool empty() const {
    return this->run([](const auto &t) { return t.empty(); });
  }

  bool contains(const std::string_view key) const {
    return this->run([key](const auto &t) { return t.contains(key); });
  }

  bool further(const std::string_view key) const {
    return this->run([key](const auto &t) { return t.further(key); });
  }

  template <class OutputIndexIterator>
  void contains_further(const std::string_view stem,
                        const std::string_view suffixes,
                        OutputIndexIterator contains_further) const {
    return this->run([=](const auto &t) {
      return t.contains_further(stem, suffixes, contains_further);
    });
  }
};

class SolverDictFactory {
  std::vector<std::string> solvers;

public:
  SolverDictFactory() {
#ifdef WORDSEARCH_SOLVER_HAS_trie
    solvers.push_back("trie");
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie
    solvers.push_back("compact_trie");
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie2
    solvers.push_back("compact_trie2");
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_vector
    solvers.push_back("dictionary_std_vector");
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_set
    solvers.push_back("dictionary_std_set");
#endif
  }

  bool has_solver(const std::string_view solver) const {
    return std::find(solvers.begin(), solvers.end(), solver) != solvers.end();
  }

  // Return type subject to change
  auto solver_names() const { return ranges::views::all(solvers); }

  template <class Words>
  auto make(const std::string_view solver, Words &&words) const {
#ifdef WORDSEARCH_SOLVER_HAS_trie
    if (solver == "trie") {
      return SolverDictWrapper{std::in_place_type<trie::Trie>,
                               std::forward<Words>(words)};
    }
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie
    if (solver == "compact_trie") {
      return SolverDictWrapper{std::in_place_type<compact_trie::CompactTrie>,
                               std::forward<Words>(words)};
    }
#endif
#ifdef WORDSEARCH_SOLVER_HAS_compact_trie2
    if (solver == "compact_trie2") {
      return SolverDictWrapper{std::in_place_type<compact_trie2::CompactTrie2>,
                               std::forward<Words>(words)};
    }
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_vector
    if (solver == "dictionary_std_vector") {
      return SolverDictWrapper{
          std::in_place_type<dictionary_std_vector::DictionaryStdVector>,
          std::forward<Words>(words)};
    }
#endif
#ifdef WORDSEARCH_SOLVER_HAS_dictionary_std_set
    if (solver == "dictionary_std_set") {
      return SolverDictWrapper{
          std::in_place_type<dictionary_std_set::DictionaryStdSet>,
          std::forward<Words>(words)};
    }
#endif
    throw std::runtime_error(fmt::format("No such solver {}", solver));
  }
};

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
                "Usage: ./this --dict path/to/dict --wordsearch path/to/ws "
                "--solver trie");

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

  const auto solvers = SolverDictFactory{};
  if (!solvers.has_solver(solver)) {
    throw std::runtime_error(fmt::format("No such solver: {}. Solvers: {}",
                                         solver, solvers.solver_names()));
  }

  const auto solver_dict = solvers.make(solver, dict_lines);

  ProfilerRestartDisabled();
  ProfilerEnable();
  const auto start = std::chrono::high_resolution_clock::now();
  const auto result = solver::solve(solver_dict, grid);
  const auto end = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "\n";
  return static_cast<int>(result.size());

}
