#include "wordsearch_solver/wordsearch_solver.hpp"

#include <cxxopts.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gperftools/profiler.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

// - Wait for 2050 when c++ has built in proper metaprogramming/static
// reflection.

int main(int argc, char **argv) {
  auto options =
      cxxopts::Options("wordsearch_solver_cmdline_app",
                       "Command line app to benchmark wordsearch_solver");

  // clang-format off
  options.add_options()
    ("d,dictionary", "Dictionary, file with list of words, one per line",
      cxxopts::value<std::string>())
    ("w,wordsearch", "Wordsearch file", cxxopts::value<std::string>())
    ("s,solver", "Dictionary solver implementation",
     cxxopts::value<std::string>())
    ("h,help", "Help")
    ;
  // clang-format on

  auto parsed_args = options.parse(argc, argv);

  if (parsed_args.arguments().empty() || parsed_args.count("help") ||
      parsed_args.count("-help") || parsed_args.count("-h") ||
      parsed_args.count("--help")) {
    std::cout << options.help() << "\n";
    return 0;
  }

  std::string dict_path;
  std::string wordsearch_path;
  std::string solver;

  // Pretty crap seem to have to define these exceptions manually, as otherwise
  // you get a useless error if pass only one of the arguments

  try {
    dict_path = parsed_args["dictionary"].as<std::string>();
  } catch (const std::domain_error &e) {
    std::cerr << "dictionary argument required"
              << "\n";
    throw;
  }

  try {
    wordsearch_path = parsed_args["wordsearch"].as<std::string>();
  } catch (const std::domain_error &e) {
    std::cerr << "wordsearch argument required"
              << "\n";
    throw;
  }

  try {
    solver = parsed_args["solver"].as<std::string>();
  } catch (const std::domain_error &e) {
    std::cerr << "solver argument required"
              << "\n";
    throw;
  }

  const auto solvers = solver::SolverDictFactory{};
  if (!solvers.has_solver(solver)) {
    throw std::runtime_error(
        fmt::format("Solver must be one of {}", solvers.solver_names()));
  }

  const auto dict_lines = utility::read_file_as_lines(dict_path);
  const auto grid =
      solver::make_grid(utility::read_file_as_lines(wordsearch_path));

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
