#ifndef WORDSEARCH_SOLVER_H
#define WORDSEARCH_SOLVER_H

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

#include "wordsearch_solver_defs.h"
#include "stringindexes.h"

namespace wordsearch_solver
{
//  using Grid = std::shared_ptr<std::vector<std::string>>;
//  using Dictionary = std::vector<std::string>;
//  using Index = std::pair<std::size_t, std::size_t>;
//  using Indexes = std::vector<Index>;

//  std::pair<std::vector<std::string>, std::vector<std::vector<Index>>>
  StringIndexes
  find_words(
      const Dictionary& dictionary, const Grid& grid,
      Index start);

  Grid grid_from_file(const std::filesystem::path& wordsearch_file);

  std::vector<std::string> readlines(const std::filesystem::path& p);

  StringIndexes solve(const Dictionary& dict, const Grid& grid);

  std::string indexes_to_word(const Grid& grid, const Indexes& tail);

  // TODO: move to .tpp or the like. Find out how to make this work with cmake
//  template<class T> void sort_unique(T& vec)
//  {
//    std::sort(vec.begin(), vec.end());
//    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
//  }

}

#endif // WORDSEARCH_SOLVER_H
