#ifndef COMPACT_TRIE_TPP
#define COMPACT_TRIE_TPP

#include "wordsearch_solver/compact_trie/compact_trie.hpp"
#include "wordsearch_solver/utility/utility.hpp"

#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/subrange.hpp>

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace compact_trie
{

// FIXME: this would use ranges::subrange as we don't need to alloc. However
// this won't compile due to std::tuple_element on incomplete class (on gcc with
// -fconcepts at least) so leaving like this for now.
template<class Iterator1, class Iterator2>
CompactTrie::CompactTrie(Iterator1 first, const Iterator2 last)
: CompactTrie(std::vector<std::string>(first, last))
{}

template<class Strings>
CompactTrie::CompactTrie(Strings&& strings_in)
: nodes_{}
, rows_{}
, size_{0}
{
  // FIXME: change this, this is here for quick dirty testing
  std::vector<std::string> strings(strings_in.begin(), strings_in.end());
  ranges::sort(strings);
  for (const auto &word : strings) {
    utility::throw_if_not_lowercase_ascii(word);
  }

  std::vector<std::size_t> rows_indexes{0};
  for (auto row: utility::words_grouped_by_prefix_suffix(strings))
  {
    // fmt::print("\nIteration\n");
    // fmt::print("data: {}\n", data);
    // const auto old_row_end = static_cast<long>(data.size());

    std::size_t bits_on = 0;
    for (auto [prefix, suffixes, is_end_of_word]: row)
    {
      size_ += is_end_of_word;
      // fmt::print("{} {} {}\n", prefix, suffixes, is_end_of_word);
      Node comp{};
      for (const char c: suffixes)
      {
        // From lowercase ascii, s.t. a -> 0, b -> 1, etc.
        comp.add_char(c);
      }
      comp.set_preceding(bits_on);
      comp.set_is_end_of_word(is_end_of_word);

      // fmt::print("{}\n", comp);

      nodes_.push_back(comp);
      bits_on += suffixes.size();
    }

    rows_indexes.push_back(nodes_.size());
  }

  rows_.reserve(rows_indexes.size());
  for (const auto index: rows_indexes)
  {
    rows_.push_back(std::next(nodes_.begin(), static_cast<long>(index)));
  }
}

template<class OutputIterator>
void CompactTrie::contains_further(const std::string_view stem,
    const std::string_view suffixes,
    OutputIterator contains_further_it) const
{
  const auto [stem_node_it, stem_rows_it] = this->search(stem,
      ranges::subrange(nodes_), ranges::subrange(rows_));
  if (stem_node_it == nodes_.end())
  {
    return;
  }

  for (const auto c: suffixes)
  {
    const std::string_view suffix = {&c, 1};
    const auto contains = this->contains(suffix,
        ranges::subrange(stem_node_it, nodes_.end()),
        ranges::subrange(stem_rows_it, rows_.end()));
    const auto further = this->further(suffix,
        ranges::subrange(stem_node_it, nodes_.end()),
        ranges::subrange(stem_rows_it, rows_.end()));
    *contains_further_it++ = {contains, further};
  }
}

}

#endif // COMPACT_TRIE_TPP
