#ifndef TRIE_TPP
#define TRIE_TPP

#include "node.hpp"
#include "trie.hpp"

#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/unique.hpp>

#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace trie
{

// FIXME: this would use ranges::subrange as we don't need to alloc. However
// this won't compile due to std::tuple_element on incomplete class (on gcc with
// -fconcepts at least) so leaving like this for now.
template<class Iterator1, class Iterator2>
Trie::Trie(Iterator1 first, const Iterator2 last)
: Trie(std::vector<std::string>(first, last))
{}

template<class Strings>
Trie::Trie(Strings&& strings_in)
: root_{}
, size_{}
// , cache_{}
{
  for (const auto& word: ranges::views::unique(strings_in))
  {
    // This returns a ptr if is real, not unique/inseted!"
    if (detail::insert(root_, word).second)
    {
      ++size_;
    }
  }
}

template<class OutputIndexIterator>
void Trie::contains_and_further(const std::string& stem,
    const std::string& suffixes,
    OutputIndexIterator contains_out_it,
    OutputIndexIterator further_out_it,
    OutputIndexIterator contains_and_further_out_it) const
{
  const auto* node = detail::search(root_, stem);
  if (!node)
  {
    return;
  }

  for (const auto [i, c]: ranges::views::enumerate(suffixes))
  {
    const std::string_view suffix = {&c, 1};
    const auto contains = detail::contains(*node, suffix);
    const auto further = detail::further(*node, suffix);
    if (contains && further)
    {
      *contains_and_further_out_it++ = i;
    } else if (contains)
    {
      *contains_out_it++ = i;
    } else if (further)
    {
      *further_out_it++ = i;
    }
  }
}

}

#endif // TRIE_TPP
