#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "matrix2d/matrix2d.hpp"
#include "wordsearch_solver/config.hpp"

#include <range/v3/view/all.hpp>
#include <range/v3/view/view.hpp>

#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
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

class SolverDictWrapper {
  std::variant<WORDSEARCH_DICTIONARY_CLASSES> t_;
  static_assert(std::is_move_constructible_v<decltype(t_)>);

  template <class Func> auto run(Func &&func) const;

public:
  template <class SolverDict, class Words>
  SolverDictWrapper(const SolverDict &solver_dict, Words &&words);

  SolverDictWrapper(SolverDictWrapper &&) = default;
  SolverDictWrapper &operator=(SolverDictWrapper &&) = default;

  std::size_t size() const;

  bool empty() const;

  bool contains(const std::string_view key) const;

  bool further(const std::string_view key) const;

  template <class OutputIndexIterator>
  void contains_further(const std::string_view stem,
                        const std::string_view suffixes,
                        OutputIndexIterator contains_further) const;
};

static_assert(std::is_move_constructible_v<SolverDictWrapper>);
static_assert(std::is_move_assignable_v<SolverDictWrapper>);

class SolverDictFactory {
  std::vector<std::string> solvers;

public:
  SolverDictFactory();

  bool has_solver(const std::string_view solver) const;

  // Return type subject to change - view onto list of strings
  // Hate how implementation detail exposing this is,
  // not sure how meant to do it else, subrange<iterator, iterator>?
  // using A = std::add_lvalue_reference_t<std::add_const_t<decltype(solvers)>>;
  // using SolverNamesView = decltype(ranges::views::all(std::declval<A>()));
  auto solver_names() const -> decltype(ranges::views::all(solvers));

  template <class Words>
  SolverDictWrapper make(const std::string_view solver, Words &&words) const;
};

} // namespace solver

#include "wordsearch_solver/solver/solver.tpp"

#endif // SOLVER_HPP
