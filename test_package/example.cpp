#include <iostream>
#include <string>
#include <vector>

#include "wordsearch_solver/wordsearch_solver.hpp"

int main() {
  std::vector<std::string> words = {
      "abcd",
      "efgh",
  };
  const auto grid = solver::make_grid(words);
  const auto trie = trie::Trie(words);
}
