#pragma once

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

namespace wordsearch_solver
{
  using Grid = std::shared_ptr<std::vector<std::string>>;
  using Dictionary = std::vector<std::string>;
  using Index = std::pair<std::size_t, std::size_t>;
  using Indexes = std::vector<Index>;

  std::pair<std::vector<std::string>, std::vector<std::vector<Index>>>
  find_words(
      const Dictionary& dictionary, const Grid& grid,
      Index start);

  Grid grid_from_file(const std::filesystem::path& wordsearch_file);

  std::vector<std::string> readlines(const std::filesystem::path& p);

  std::vector<std::string> solve(
      const Dictionary& dict, const Grid& grid);

  std::pair<
    std::vector<std::vector<std::string>>,
    std::vector<std::vector<std::vector<Index>>>
  >
  solve_words_indexes(
      const Dictionary& dict, const Grid& grid);

  //  TODO: move to .tpp or the like. Find out how to make this work with cmake
  template<class T> void sort_unique(T& vec)
  {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
  }

}
