#include "wordsearch_solver/solver/solver.hpp"
#include "matrix2d/matrix2d.hpp"
#include "wordsearch_solver/config.hpp"

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/primitives.hpp>
#include <range/v3/view/all.hpp>

#include <algorithm>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace solver {

WordsearchGrid make_grid(const std::vector<std::string> &lines) {

  const auto longest_word = ranges::accumulate(
      lines,
      std::remove_reference_t<decltype(lines)>::size_type{0},
      [] (const auto a, const auto b) { return ranges::max(a, b); },
      ranges::size);

  WordsearchGrid grid(lines.size(), longest_word);
  auto rows_iter = grid.rows_iter();
  auto it = rows_iter.begin();
  [[maybe_unused]] const auto last = rows_iter.end();
  for (const auto& line: lines) {
    assert(it != last);
    std::copy(line.begin(), line.end(), it->begin());
    ++it;
  }
  return grid;
}

std::size_t SolverDictWrapper::size() const {
  return this->run([](const auto &t) { return t.size(); });
}

bool SolverDictWrapper::empty() const {
  return this->run([](const auto &t) { return t.empty(); });
}

bool SolverDictWrapper::contains(const std::string_view key) const {
  return this->run([key](const auto &t) { return t.contains(key); });
}

bool SolverDictWrapper::further(const std::string_view key) const {
  return this->run([key](const auto &t) { return t.further(key); });
}

SolverDictFactory::SolverDictFactory() {
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

bool SolverDictFactory::has_solver(const std::string_view solver) const {
  return std::find(solvers.begin(), solvers.end(), solver) != solvers.end();
}

// Return type subject to change
auto SolverDictFactory::solver_names() const
    -> decltype(ranges::views::all(solvers)) {
  return ranges::views::all(solvers);
}

} // namespace solver
