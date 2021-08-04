#include "wordsearch_solver/compact_trie2/empty_node_view.hpp"

#include "wordsearch_solver/compact_trie2/compact_trie2_iterator_typedefs.hpp"

namespace compact_trie2
{

// Explicit template instantiations
template class EmptyNodeView_<DataIterator>;
template class EmptyNodeView_<DataIteratorMut>;

}
