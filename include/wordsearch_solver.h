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
//#include "dictionary.h"
#include <nonstd/span.hpp>

namespace wordsearch_solver
{
inline Grid grid_from_file(const std::filesystem::path& wordsearch_file);

inline std::string indexes_to_word(const Grid& grid,
    const nonstd::span<Index, nonstd::dynamic_extent> tail);

template<class Dictionary>
StringIndexes solve(const Dictionary& dict, const Grid& grid);

template<class Dictionary>
__attribute__((__noinline__))
StringIndexes find_words(
    const Dictionary& dictionary, const Grid& grid, const Index start);

}

// Should be a .tpp or something
// Just doing this to get new version up and running for now
// TODO: change this!
#include "wordsearch_solver.tcc"

#endif // WORDSEARCH_SOLVER_H
