#ifndef COMPACT_TRIE2_COMPACT_TRIE2_HPP
#define COMPACT_TRIE2_COMPACT_TRIE2_HPP

#include "wordsearch_solver/compact_trie2/compact_trie2_iterator_typedefs.hpp"
#include "wordsearch_solver/compact_trie2/empty_node_view.hpp"
#include "wordsearch_solver/compact_trie2/full_node_view.hpp"
#include "wordsearch_solver/utility/flat_char_value_map.hpp"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/** namespace compact_trie2 */
namespace compact_trie2 {

/** Variable sized inline contiguous trie to allow 1 byte sized only end of word
 * nodes and full nodes.
 *
 * In the compact_trie, every node is a fixed size (happens to be 16 bytes
 * currently on my system x64 libstdc++8). There must be at least as many nodes
 * as words, roughly ~115k in the (slightly altered) GNU English dictionary used
 * for this. There may be more, if for example a node is required to represent
 * child nodes, but is not a word itself.
 *
 * For example, the prefix "abj" is not a word, but would need a node in the
 * compact_trie trie, to allow it to link to other words that have "abj" as a
 * prefix, such as "abject".
 *
 * The idea with compact_trie2 was to try and optimise this further, to allow
 * for two types of node.
 *
 * In this trie, there are empty nodes and full ones.
 * * Full nodes are like the nodes in any other trie, they can represent the end
 * of a word, plus have links to child nodes.
 * * Empty nodes are one byte, and represent only the end of a word.
 *
 * The hope was a tradeoff in saving space would offset the additional lookup
 * complexity in this data structure. A complication that arises is the variable
 * sizes of nodes, offset calcuation is more complex.
 *
 * The idea with this trie was to have everything inline in one data structure.
 * Unfortunately, performance tends to be only slightly better than the
 * (pointer) trie, and worse than the compact_trie. It has to do more work at
 * each node to calculate the offset to the next one. Also for reasons I haven't
 * yet looked into, on the reference dictionary in
 * test/test_cases/dictionary.txt, the compact_trie generates a 280kB~ trie,
 * whereas this is around 1.1MB. TODO: look into this (would explain the rubbish
 * performance).
 *
 * ```
 * Example layout for a node that has two children
 * Node size is byte at A (size) * 3 + 2.
 * |____| = 1 byte
 *
 *                         'a'   'c'
 * |____||____||____||____||____||____||____||____||____||____|
 *  A     B     C     DE    F     G     H     H     I     I
 *
 * - A: 1 byte, size, number of characters in this node
 * - B, C, D: Bytes B and C plus all but the highest bit in D make up the
 * offset, from the start of the next row, where this node's children begin.
 * - E: Highest bit in byte D. Flag to signal whether this node represents
 * 	the end of a word
 * - F, G: Sorted 1 byte entries of child nodes.
 * 	Ie. F may be the ascii corresponding to 'a', G == 'c'
 * - H, I: Corresponding child intra-node offsets, from the next row node offset
 * 	given by BCD (with highest bit zeroed).
 * Ie. to find the child node from the example node for the letter 'c', get the
 * 	position of the start of the next row,
 * 	add BCD (without highest bit) and add the letter offset in the 2 byte
 * int I.
 * ```
 *
 * @note Member functions in this class that could be const aren't because the
 * EmptyNodeView and FullNodeView classes wrap iterators. These iterators are
 * into data_'s type (std::vector<std::uint8_t>). However FullNodeView
 * requires a mutable iterator (at least whilst building the data structure)
 * as it mutates underlying data. But then if you try to call this->search()
 * where search is marked as a const member, you can't because trying to make
 * a std::variant<EmptyNodeView, FullNodeView> where each holds a (mutable)
 * std::vector<std::uint8_t>::iterator won't work as you are in a const member
 * function and therefore pass them std::vector<std::uint8_t>::const_iterator.
 *
 */
class CompactTrie2 {
public:
  CompactTrie2() = default;

  CompactTrie2(CompactTrie2&&) = default;
  CompactTrie2& operator=(CompactTrie2&&) = default;

  CompactTrie2(const CompactTrie2&) = delete;
  CompactTrie2& operator=(const CompactTrie2&) = delete;

  CompactTrie2(const std::initializer_list<std::string_view>& words);
  CompactTrie2(const std::initializer_list<std::string>& words);
  CompactTrie2(const std::initializer_list<const char*>& words);

  template <class Iterator1, class Iterator2>
  CompactTrie2(Iterator1 first, const Iterator2 last);

  // TODO: awful SFINAE or wait until 2030 for widespread cpp20 concepts to
  // constrain this to a ForwardRange
  /** Actual constructor that does the work
   *
   * @param[in] words Must be at least a forward range. This function needs to
   * sort the data passed to it, and tries to accept views and containers. If
   * @p words is @b not sorted, and is an rvalue, it will be sorted in place and
   * no copy made. Else if @p words is @b not sorted and is not an rvalue, or it
   * is a view, a copy of it will be made.
   */
  template <class ForwardRange> explicit CompactTrie2(ForwardRange&& words);

  std::size_t size() const;

  /** Size of underlying data store in bytes. */
  std::size_t data_size() const;

  bool empty() const;

  /** @copydoc solver::SolverDictWrapper::contains() */
  bool contains(const std::string_view word) const;

  /** @copydoc solver::SolverDictWrapper::further() */
  bool further(const std::string_view word) const;

  /** @copydoc solver::SolverDictWrapper::contains_further() */
  template <class OutputIterator>
  void contains_further(const std::string_view stem,
                        const std::string_view suffixes,
                        OutputIterator contains_further_it) const;

  friend std::ostream& operator<<(std::ostream& os, const CompactTrie2& ct);

private:
  /** Construction helper, as may be given a variety of types of input. */
  template <class T> void init(const T& words_view);

  /** Rest of constructor, called by init().
   * @note This is in compact_trie2.cpp rather than compact_trie2.tpp. Feels
   * kind of messy but I can't think of a much nicer alternative.
   */
  void non_templated_rest_of_init();

  /** Search through @p word as far as possible, starting at the node @p it in
   * the row @p rows_it. If the trie contains @p word, then the returned
   * `std::size_t` will be equal to @p word.size().
   * Similar to compact_trie::CompactTrie::search()
   *
   * @param[in] word The word to search for
   * @param[in] it The node to start at
   * @param[in] rows_it The row @p it is in
   * @returns A tuple: the number of letters found, the node corresponding to
   * the last letter found, the row that node is in.
   */
  std::tuple<std::size_t, DataIterator, RowIterator>
  search(const std::string_view word, DataIterator it,
         RowIterator rows_it) const;

  using ContiguousContainer = std::vector<std::uint8_t>;
  using ContiguousContainerIterator = ContiguousContainer::iterator;

  ContiguousContainer data_;
  std::vector<ContiguousContainerIterator> rows_;
  std::size_t size_;
  mutable utility::FlatCharValueMap<std::pair<DataIterator, RowIterator>>
      cache_;
};

} // namespace compact_trie2

#include "wordsearch_solver/compact_trie2/compact_trie2.tpp"

#endif // COMPACT_TRIE2_COMPACT_TRIE2_HPP
