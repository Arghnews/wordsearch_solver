#ifndef TRIE_HPP
#define TRIE_HPP

#include "wordsearch_solver/trie/node.hpp"
#include "wordsearch_solver/utility/flat_char_value_map.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// TODO: test this with const_iterator not a std::tuple, and try a simple user
// defined struct/pointer to make trivial type to help the optimiser? Not even
// sure if "trivial" means what I think it does anyway, remove this likely..
// TODO: maybe look into units library for the ascii/index conversion stuff, as
// that has already wasted a significant amount of time with offset stuff
// TODO: std::bitset size on this system is 8 bytes, even though it need only be
// 4 bytes (26 bits) for lowercase ascii. Could see if writing own node without
// std::bitset that is 4 bytes big changes performance due to whole thing being
// approx half size (better for cache).

namespace trie {

/** Recursive immutable node based trie.
 *
 * Recursive tree structure of nodes, where each node holds a vector-like
 * container of edges, and each edge consists of a character and a pointer to
 * the corresponding child node.
 * To lookup a word of length "m", using a dictionary with "d" distinct
 * characters, for example d == 26 for lowercase ascii and the English alphabet,
 * lookup is O(m * d).
 *
 * Realistically, the factor of d will usually be much less
 * than the actual value of d, so really more like just O(m). Could say that
 * furthermore, since (in English at least) average word length is much shorter
 * than max(m) anyway, essentially this becomes almost constant time lookup.
 *
 * @note Not thread safe even for just reading, due to use of unprotected
 * internal cache.
 */
class Trie {
public:
  Trie() = default;

  Trie(Trie&&) = default;
  Trie& operator=(Trie&&) = default;

  /** @note See trie/include/wordsearch_solver/trie/node.hpp before
   * changing/implementing this, must implement proper deep copy, traits don't
   * behave nicely.
   */
  Trie(const Trie&) = delete;
  Trie& operator=(const Trie&) = delete;

  Trie(const std::initializer_list<std::string_view>& words);
  Trie(const std::initializer_list<std::string>& words);
  Trie(const std::initializer_list<const char*>& words);

  template <class Iterator1, class Iterator2>
  Trie(Iterator1 first, const Iterator2 last);

  // TODO: constrain this (sfinae or concepts(>=c++20))
  // Strings should be a range of strings
  template <class Strings> explicit Trie(Strings&& strings_in);

  /** Check whether this dictionary contains a word.
   *
   * @param[in] word
   */
  bool contains(std::string_view word) const;

  /** Check whether this dictionary may contain words that start with prefix @p
   * word.
   *
   * @param[in] word
   * @return `false` when there are no words that have @p word as a stem (and
   * are at least 1 character longer than @p word). `true` if there may be more
   * words with @p word as a stem.
   *
   * @note This may return `true` for some dictionary solver implementations
   * even when there are in fact no more words with @p word as a stem remaining
   * \- the naive solvers using `std::set` and `std::vector`, notably. As long
   * as it eventually returns `false` when there are no further words with this
   * prefix, this is acceptable, though suboptimal.
   */
  bool further(std::string_view word) const;

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
  template <class OutputIterator>
  void contains_further(const std::string_view stem,
                        const std::string_view suffixes,
                        OutputIterator contains_further_it) const;

  std::size_t size() const;
  bool empty() const;

  friend std::ostream& operator<<(std::ostream& os, const Trie& ct);

private:
  const Node* search(std::string_view word) const;
  std::pair<Node*, bool> insert(std::string_view word);

  Node root_;
  std::size_t size_;
  mutable utility::FlatCharValueMap<const Node*> cache_;
};

namespace detail {

bool contains(const Node& node, std::string_view word);
bool further(const Node& node, std::string_view word);

} // namespace detail

} // namespace trie

#include "wordsearch_solver/trie/trie.tpp"

#endif // TRIE_HPP
