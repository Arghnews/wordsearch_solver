#include "wordsearch_solver/compact_trie2/full_node_view.hpp"

#include "wordsearch_solver/compact_trie2/compact_trie2_iterator_typedefs.hpp"

namespace compact_trie2 {

// Explicit template instantiations
template class FullNodeView_<DataIterator>;
template class FullNodeView_<DataIteratorMut>;

} // namespace compact_trie2
