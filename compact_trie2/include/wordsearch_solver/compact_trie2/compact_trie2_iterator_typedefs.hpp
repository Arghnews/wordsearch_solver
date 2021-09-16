#ifndef COMPACT_TRIE2_COMPACT_TRIE2_ITERATOR_TYPEDEFS_HPP
#define COMPACT_TRIE2_COMPACT_TRIE2_ITERATOR_TYPEDEFS_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace compact_trie2 {

using DataContainer = std::vector<std::uint8_t>;
using DataIterator = DataContainer::const_iterator;
using DataIteratorMut = DataContainer::iterator;
using RowContainer = std::vector<DataIteratorMut>;
using RowIterator = RowContainer::const_iterator;
using RowIteratorMut = RowContainer::iterator;

} // namespace compact_trie2

#endif
