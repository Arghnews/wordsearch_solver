#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "matrix2d/matrix2d.hpp"
#include "wordsearch_solver/config.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace solver
{

using Index = matrix2d::Index;
using Tail = std::vector<Index>;
using ListOfListsOfIndexes = std::vector<Tail>;
using WordToListOfListsOfIndexes =
    std::unordered_map<std::string, ListOfListsOfIndexes>;
// Would like to use heterogenous map here to be able to lookup using
// string_view's, but even if use a custom hash with string/string_view
// overloads on operator(), and std::equal_to<void> template args, the
// std::unordered_map.find() templated version is c++20
using WordsearchGrid = matrix2d::Matrix2d<char>;

template <class SolverDict>
void solve_index(const SolverDict &solver_dict, const WordsearchGrid &grid,
                 const Index start_index,
                 WordToListOfListsOfIndexes &word_to_list_of_indexes);

template <class SolverDict>
WordToListOfListsOfIndexes solve(const SolverDict &solver_dict,
                                 const WordsearchGrid &grid);

WordsearchGrid make_grid(const std::vector<std::string> &lines);
}

#include "wordsearch_solver/solver/solver.tpp"

#endif // SOLVER_HPP
