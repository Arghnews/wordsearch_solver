#ifndef TRIE_TPP
#define TRIE_TPP

#include "wordsearch_solver/trie/node.hpp"
#include "wordsearch_solver/trie/trie.hpp"
#include "wordsearch_solver/utility/utility.hpp"

#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/unique.hpp>

#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace trie {

// FIXME: this would use ranges::subrange as we don't need to alloc. However
// this won't compile due to std::tuple_element on incomplete class (on gcc with
// -fconcepts at least) so leaving like this for now.
template <class Iterator1, class Iterator2>
Trie::Trie(Iterator1 first, const Iterator2 last)
    : Trie(std::vector<std::string>(first, last)) {}

/** The constructor that actually does the work */
template <class Strings>
Trie::Trie(Strings&& strings_in) : root_{}, size_{}, cache_{} {
  for (const auto& word : ranges::views::unique(strings_in)) {
    if (this->insert(word).second) {
      ++size_;
    }
  }
}

template <class OutputIterator>
void Trie::contains_further(const std::string_view stem,
                            const std::string_view suffixes,
                            OutputIterator contains_further_it) const

{
  const auto* node = this->search(stem);
  if (!node) {
    return;
  }

  for (const auto [i, c] : ranges::views::enumerate(suffixes)) {
    const std::string_view suffix = {&c, 1};
    const auto contains = detail::contains(*node, suffix);
    const auto further = detail::further(*node, suffix);
    *contains_further_it++ = {contains, further};
  }
}

} // namespace trie

#endif // TRIE_TPP
