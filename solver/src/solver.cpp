#include "wordsearch_solver/solver/solver.hpp"
#include "matrix2d/matrix2d.hpp"

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/primitives.hpp>

#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

solver::WordsearchGrid
solver::make_grid(const std::vector<std::string> &lines) {

  const auto longest_word = ranges::accumulate(
      lines,
      std::remove_reference_t<decltype(lines)>::size_type{0},
      [] (const auto a, const auto b) { return ranges::max(a, b); },
      ranges::size);

  WordsearchGrid grid(lines.size(), longest_word);
  auto rows_iter = grid.rows_iter();
  auto it = rows_iter.begin();
  const auto last = rows_iter.end();
  for (const auto& line: lines) {
    assert(it != last);
    std::copy(line.begin(), line.end(), it->begin());
    ++it;
  }
  return grid;
}
