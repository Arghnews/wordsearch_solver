#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "matrix2d/matrix2d.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace solver
{

using Index = matrix2d::Index;
using Tail = std::vector<Index>;
using ListOfListsOfIndexes = std::vector<Tail>;
using WordToListOfListsOfIndexes =
    std::unordered_map<std::string, ListOfListsOfIndexes,
                       std::hash<std::string>, std::equal_to<void>>;
using WordsearchGrid = matrix2d::Matrix2d<char>;

template <class Dict>
void solve_index(const Dict &dict, const WordsearchGrid &grid,
                 const Index start_index,
                 WordToListOfListsOfIndexes &word_to_list_of_indexes);

template <class Dict>
WordToListOfListsOfIndexes solve(const Dict &dict, const WordsearchGrid &grid);

WordsearchGrid make_grid(const std::vector<std::string> &lines);
}

#include "wordsearch_solver/solver/solver.tpp"

#endif // SOLVER_HPP
