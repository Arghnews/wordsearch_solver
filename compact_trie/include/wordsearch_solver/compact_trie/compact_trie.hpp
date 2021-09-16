#ifndef COMPACT_TRIE_HPP
#define COMPACT_TRIE_HPP

#include "wordsearch_solver/compact_trie/node.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <range/v3/view/subrange.hpp>

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

// This might ACTUALLY be a case for inheritance what with the CompactTrie being
// a Trie?
// TODO: test this with const_iterator not a std::tuple, and try a simple user
// defined struct/pointer to make trivial type to help the optimiser? Not even
// sure if "trivial" means what I think it does anyway, remove this likely..
// TODO: maybe look into units library for the ascii/index conversion stuff, as
// that has already wasted a significant amount of time with offset stuff
// TODO: std::bitset size on this system is 8 bytes, even though it need only be
// 4 bytes (26 bits) for lowercase ascii. Could see if writing own node without
// std::bitset that is 4 bytes big changes performance due to whole thing being
// approx half size (better for cache).

namespace compact_trie {

/** Inline contiguous immutable trie.
 *
 * Based on the trie::Trie, but now everything's inline, and the whole thing is
 * in one big contiguous memory block.
 *
 * @note This (currently) only works for lowercase ascii, as internally a
 * `std::bitset<26>` is used to represent child nodes. Also, libstdc++ 8 uses a
 * minimum of one `unsigned long`, 8 bytes, to represent even this, when 4 bytes
 * would do (potential optimisation rolling own bitset etc?).
 *
 * A node consists of a bitset of size 26 (bits), with each bit representing an
 * edge to a child node and a letter. A node also has a bool to indicate whether
 * it's a word end, and an int to indicate how many nodes before this one on the
 * same row existed, which is required for calculating the offset in the next
 * row of the child nodes. We also keep a track of indexes into said vector of
 * nodes that correspond to each row.  A row corresponds to all the letters at
 * that position in a word.
 *
 * Lookup for a word of length "m" is similar to the trie, however now each
 * lookup for every letter of a word, rather than a linear search through a
 * small list of letters, is a lookup for a bit being on in a bitset. O(n) in
 * the number of child nodes or letters from that stem, to O(1), a simple
 * bitmask.
 *
 * Then, assuming that letter is present, read off from the node the offset on
 * the next row, where that letter's next node is found. Lookup for a word of
 * length m is O(m), plus likely better cache locality (unless change trie to
 * use a different allocator, which should be possible as can precompute
 * size from input, but then you basically end up making this anyway).
 *
 */
class CompactTrie {
public:
  using Nodes = std::vector<Node>;
  using NodesIterator = std::vector<Node>::const_iterator;
  using Rows = std::vector<NodesIterator>;
  using RowsIterator = Rows::const_iterator;
  using const_iterator = std::tuple<NodesIterator, RowsIterator>;
  // static_assert(std::is_trivially_copyable_v<const_iterator>);

  CompactTrie() = default;

  CompactTrie(CompactTrie&&) = default;
  CompactTrie& operator=(CompactTrie&&) = default;

  CompactTrie(const CompactTrie&) = delete;
  CompactTrie& operator=(const CompactTrie&) = delete;

  CompactTrie(const std::initializer_list<std::string_view>& words);
  CompactTrie(const std::initializer_list<std::string>& words);
  CompactTrie(const std::initializer_list<const char*>& words);

  template <class Iterator1, class Iterator2>
  CompactTrie(Iterator1 first, const Iterator2 last);

  /** Actual constructor, all other delegate to this. */
  template <class Strings> explicit CompactTrie(Strings&& strings_in);

  /** @copydoc trie::Trie::contains() */
  bool contains(std::string_view word) const;
  /** @copydoc trie::Trie::further() */
  bool further(std::string_view word) const;

  /** @copydoc trie::Trie::contains_further() */
  template <class OutputIterator>
  void contains_further(std::string_view stem, std::string_view suffixes,
                        OutputIterator contains_further_it) const;

  std::size_t size() const;
  bool empty() const;

  friend std::ostream& operator<<(std::ostream& os, const CompactTrie& ct);

private:
  bool contains(std::string_view word, ranges::subrange<NodesIterator> nodes,
                ranges::subrange<RowsIterator> rows) const;

  bool further(std::string_view word, ranges::subrange<NodesIterator> nodes,
               ranges::subrange<RowsIterator> rows) const;

  /** Search as far as possible for @p word.
   * @param[in] word Word to find
   * @param[in] nodes Range of nodes in which to search
   * @param[in] rows Range of rows
   * @returns An iterator into @p nodes and an iterator into @p rows.
   *
   * Search as far as possible for @p word in @p nodes given @p rows.
   *
   * Returns `{it, rows_it}`.
   * If @p word is found, `std::distance(rows.begin(), rows_it) == word.size()`
   * Else it will the deepest node/letter present in the dictionary, in @p word
   */
  CompactTrie::const_iterator
  search(std::string_view word,
         ranges::subrange<CompactTrie::NodesIterator> nodes,
         ranges::subrange<CompactTrie::RowsIterator> rows) const;

  Nodes nodes_;
  Rows rows_;
  std::size_t size_;
};

} // namespace compact_trie

#include "wordsearch_solver/compact_trie/compact_trie.tpp"

#endif // COMPACT_TRIE_HPP
