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

/** Classes to solve a wordsearch.
 *
 * See example usage as in example/example.cpp
 *
 * @include example.cpp
 *
 */
namespace solver {

/** 2 element coordinate */
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

/** Find all possible words using @p solver_dict that may start at @p
 * start_index in @p grid, and write them out to @p word_to_list_of_indexes
 *
 * @param[in] solver_dict The solver dictionary implementation to use, for
 * example SolverDictWrapper
 * @param[in] grid The wordsearch matrix/grid to solve
 * @param[in] start_index The start/first letter of the word to solve
 * @param[out] word_to_list_of_indexes The map from words to lists of indexes
 * that results are written out to
 */
template <class SolverDict>
void solve_index(const SolverDict& solver_dict, const WordsearchGrid& grid,
                 const Index start_index,
                 WordToListOfListsOfIndexes& word_to_list_of_indexes);

/** Runs solve_index() on every element of @p grid to solve the whole wordsearch
 * @param[in] solver_dict The solver dictionary implementation to use, for
 * example SolverDictWrapper
 * @param[in] grid The wordsearch matrix/grid to solve
 * @returns The map from words to lists of indexes that results are written out
 * to
 */
template <class SolverDict>
WordToListOfListsOfIndexes solve(const SolverDict& solver_dict,
                                 const WordsearchGrid& grid);

/** Helper function to construct a `WordsearchGrid` */
WordsearchGrid make_grid(const std::vector<std::string>& lines);

/** A type erased wrapper around a particular solver dictionary implementation.
 * Instances of this should be constructed by SolverDictFactory::make()
 *
 * @note Uses `std::variant` for type erasure internally, so will need to be
 * recompiled if the list of available solver types in the macro
 * WORDSEARCH_DICTIONARY_CLASSES changes.
 */
class SolverDictWrapper {
  std::variant<WORDSEARCH_DICTIONARY_CLASSES> t_;
  static_assert(std::is_move_constructible_v<decltype(t_)>);

  template <class Func> auto run(Func&& func) const;

public:
  template <class SolverDict, class Words>
  SolverDictWrapper(const SolverDict& solver_dict, Words&& words);

  SolverDictWrapper(SolverDictWrapper&&) = default;
  SolverDictWrapper& operator=(SolverDictWrapper&&) = default;

  /** The number of whole words in this dictionary */
  std::size_t size() const;

  /** Checks if this dictionary is empty */
  bool empty() const;

  /** Check if this dictionary contains @p word
   *
   * @param[in] word The word to check
   * @returns `true` if @p word is present, else `false`
   */
  bool contains(const std::string_view word) const;

  /** Check if this dictionary @b might contain words with @p word as a prefix
   *
   * @param[in] word The prefix to check
   * @returns `false` if there are no more strings in the dictionary with @p
   * word as a prefix, `true` if there might be.
   *
   * @note Some solvers do not conclusively know whether, for a prefix @p word,
   * there are more words that contain that prefix. This is acceptable, though
   * sub-optimal and will result in wasted search time, as long as eventually
   * this returns false, assuming for @p word there are in fact no more words
   * with that prefix.
   */
  bool further(const std::string_view word) const;

  /** For each char in suffix appended to stem, check whether this dictionary
   * contains this word and if it may contain longer words with this prefix.
   *
   * @param[in] stem
   * @param[in] suffixes
   * @param[out] contains_further_it
   *
   * This function is what the solver algorithm calls every iteration to ask the
   * dictionary solver implementation to do its work.
   *
   * @p contains_further_it should be assigned to and incremented like an
   * output iterator. The value written should be a std::pair<bool, bool>.
   *
   * Example contains_further implementation:
   * @code
   * const auto* node = this->search(stem);
   * if (!node) {
   *   return;
   * }
   *
   * for (const auto [i, c] : ranges::views::enumerate(suffixes)) {
   *   const std::string_view suffix = {&c, 1};
   *   const auto contains = detail::contains(*node, suffix);
   *   const auto further = detail::further(*node, suffix);
   *   *contains_further_it++ = {contains, further};
   * }
   * @endcode
   *
   * Each character's position in @p suffix corresponds to the order that
   * that @p suffix's output should be written to @p contains_further_it.
   *
   * @p suffixes is @b not guaranteed to be sorted.
   */
  template <class OutputIndexIterator>
  void contains_further(const std::string_view stem,
                        const std::string_view suffixes,
                        OutputIndexIterator contains_further_it) const;
};

static_assert(std::is_move_constructible_v<SolverDictWrapper>);
static_assert(std::is_move_assignable_v<SolverDictWrapper>);

/** This class can be used to check if a particular dictionary solver
 * implementation exists, and create an instance of one with a particular
 * dictionary.
 *
 * See example/example.cpp for how to use it
 *
 * @note The implementation behind this sucks. All the information required is
 * available at compile time, namely what solver implementations were compiled.
 * However, I cannot find a way to implement the interface to this class like
 * this, and SolverDictWrapper like it is, if the solvers are stored in
 * something like a `boost::hana::tuple`. Ie. I want to store the immutable data
 * in a compile time data structure like a `boost::hana::tuple`, but be able to
 * query it using runtime parameters like `std::string`s.
 * As a result, this uses a `std::vector<std::string>` and some filthy macros
 * instead.
 */
class SolverDictFactory {
  std::vector<std::string> solvers;

public:
  SolverDictFactory();

  /** Check if a dictionary solver implementation called @p solver exists
   * @param[in] solver The name of the dictionary solver implementation
   * @returns A bool indicating whether a @p solver with that name xists
   */
  bool has_solver(const std::string_view solver) const;

  /** A view onto the names of available solvers
   *
   * Return type leaks implementation details, I dislike it but am unsure how to
   * avoid it
   */
  auto solver_names() const -> decltype(ranges::views::all(solvers));

  /** Make a dictionary solver
   *
   * @param[in] solver Name of the dictionary solver implementation
   * @param[in] dictionary List of words to use as a dictionary
   * @returns An initialised SolverDictWrapper
   */
  template <class Words>
  SolverDictWrapper make(const std::string_view solver,
                         Words&& dictionary) const;
};

} // namespace solver

#include "wordsearch_solver/solver/solver.tpp"

#endif // SOLVER_HPP
