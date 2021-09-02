#include "wordsearch_solver/wordsearch_solver.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

int main() {
  // Define a wordsearch
  // clang-format off
  const std::vector<std::string> wordsearch = {
    "adg",
    "beh",
    "cfi",
  };
  // clang-format on

  // Define a container of string-like objects that is the dictionary
  const std::initializer_list<std::string_view> dictionary = {"zoo", "badge",
                                                              "be", "beg"};
  // const auto dictionary = utility::read_file_as_lines("path/to/dict");

  const auto solver = solver::SolverDictFactory{}.make("trie", dictionary);
  const auto grid = solver::make_grid(wordsearch);

  // Solve the wordsearch
  const solver::WordToListOfListsOfIndexes answer = solver::solve(solver, grid);

  for (const auto& [word, indexes] : answer) {
    fmt::print("{}: {}\n", word, indexes);
  }
  // [0, 0] is top left, 'a'. [1, 0] is 'b'. [2, 1] is 'f'.
  // Prints:
  // beg: {{[1, 0], [1, 1], [0, 2]}}
  // be: {{[1, 0], [1, 1]}}
  // badge: {{[1, 0], [0, 0], [0, 1], [0, 2], [1, 1]}}
}
