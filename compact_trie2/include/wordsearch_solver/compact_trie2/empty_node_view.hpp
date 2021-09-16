#ifndef EMPTY_NODE_VIEW_HPP
#define EMPTY_NODE_VIEW_HPP

#include "wordsearch_solver/compact_trie2/compact_trie2_iterator_typedefs.hpp"

#include <cassert>
#include <cstdint>
#include <ostream>
#include <vector>

namespace compact_trie2 {

/** An empty node is simply a single std::uint8_t (byte) with value 0.
 */
template <class Iterator> class EmptyNodeView_ {
public:
  // Unsure if constexpr makes any difference to member functions here
  constexpr explicit EmptyNodeView_(Iterator it) : it_(it) { assert(*it == 0); }

  /** @returns The underlying iterator */
  constexpr auto base() const { return it_; }

  /** @returns The size in bytes of the node */
  constexpr std::size_t size() const { return 1; }

  /** @returns Trivially true for an EmptyNodeView_ */
  constexpr bool is_only_end_of_word() const { return true; }

  /** @returns The size in bytes of the data section of this node, 0 for
   * EmptyNodeView_ */
  constexpr std::uint8_t data_size() const {
    // gcc complains that the iterator's operator* here isn't constexpr
    // clang doesn't
    // assert(*it_ == 0);
    return 0;
  }

  /** @returns Trivially true for an EmptyNodeView_ */
  constexpr bool is_end_of_word() const { return true; }

  friend std::ostream& operator<<(std::ostream& os, const EmptyNodeView_&) {
    return os << "EmptyNode";
  }

  Iterator it_;
};

using EmptyNodeViewMut = EmptyNodeView_<DataIteratorMut>;
using EmptyNodeView = EmptyNodeView_<DataIterator>;

} // namespace compact_trie2

#endif // EMPTY_NODE_VIEW_HPP
