#include "wordsearch_solver/compact_trie2/compact_trie2.hpp"

#include "wordsearch_solver/utility/utility.hpp"
#include "wordsearch_solver/compact_trie2/compact_trie2_iterator_typedefs.hpp"
#include "wordsearch_solver/compact_trie2/empty_node_view.hpp"
#include "wordsearch_solver/compact_trie2/full_node_view.hpp"

#include <range/v3/iterator/operations.hpp>
#include <range/v3/range/access.hpp>
#include <range/v3/view/adaptor.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/view/transform.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

// FIXME: Remove this
#include <iostream>

namespace compact_trie2 {

CompactTrie2::CompactTrie2(const std::initializer_list<std::string_view>& words)
    : CompactTrie2(ranges::views::all(words)) {}

CompactTrie2::CompactTrie2(const std::initializer_list<std::string>& words)
    : CompactTrie2(ranges::views::all(words)) {}

CompactTrie2::CompactTrie2(const std::initializer_list<const char*>& words)
    : CompactTrie2(ranges::views::all(words) |
                   ranges::views::transform([](const auto string_literal) {
                     return std::string_view{string_literal};
                   })) {}

// Member functions in this class that could be const aren't because the
// EmptyNodeView and FullNodeView classes wrap iterators. These iterators are
// into data_'s type (std::vector<std::uint8_t>). However FullNodeView
// requires a mutable iterator (at least whilst building the data structure)
// as it mutates underlying data. But then if you try to call this->search()
// where search is marked as a const member, you can't because trying to make
// a std::variant<EmptyNodeView, FullNodeView> where each holds a (mutable)
// std::vector<std::uint8_t>::iterator won't work as you are in a const member
// function and therefore pass them std::vector<std::uint8_t>::const_iterator.

namespace {

// View onto nodes representation, where the underlying is a contiguous sequence
// of std::uint8_t s
// This view adaptor is based on
// http://ericniebler.github.io/range-v3/ section Utilities, Create Custom Views
// with view_adaptor
template <typename Rng>
class NodeIteratorRange
    : public ranges::view_adaptor<NodeIteratorRange<Rng>, Rng> {
  friend ranges::range_access;

  using base_iterator_t = ranges::iterator_t<Rng>;

  // Iterator adaptor for this range so that incrementing iterators respects the
  // underlying node size and moves by the correct distance
  struct query_adaptor : public ranges::adaptor_base {
    query_adaptor() = default;

    query_adaptor(const NodeIteratorRange&) {}

    auto read(base_iterator_t const& it) const { return it; }

    // This is the only interesting/non-boilerplate thing in this class, we need
    // next to increment the underlying iterator according to the size of the
    // node
    void next(base_iterator_t& it) {
      // Overkill for this when we can just read *it (vs node_size(it))
      // Leaving for now until we either have a nicer abstraction (reading *it
      // arguably breaks encapsulation somewhat) or this proves to be a
      // performance problem.
      ranges::advance(it, static_cast<long>(node_size(it)));
    }
  };

public:
  NodeIteratorRange() = default;

  NodeIteratorRange(Rng&& rng)
      : NodeIteratorRange::view_adaptor{std::forward<Rng>(rng)} {}

  auto begin_adaptor() const { return query_adaptor{}; }

  auto end_adaptor() const { return query_adaptor{}; }
};

} // namespace

// Splitting the init method into a templated init in the header and then this
// in the cpp allows this large function not to have to be in the header, and
// also allows the NodeIteratorRange class not to have to be in the header.
// Originally this was split like this for compile time improvements.
// I am unsure whether or not these motivations justify this rather unneat
// but pragmatic way of doing this.
void CompactTrie2::non_templated_rest_of_init() {

  // Each parent row, or row above, row0 construction depends on the child row
  // below it, row1.
  const auto& rows = rows_;
  for (auto&& [row0, row1] : make_adjacent_pairwise_rows_view(
           ranges::views::all(data_), ranges::views::all(rows))) {

    // fmt::print("\nNew row pair iteration\n");
    auto row0_range = NodeIteratorRange{ranges::views::all(row0)};
    auto row1_range = NodeIteratorRange{ranges::views::all(row1)};

    auto row1_it = row1_range.begin();
    [[maybe_unused]] const auto row1_end = row1_range.end();

    std::size_t offset = 0;
    for (auto row0_it = row0_range.begin(); row0_it != row0_range.end();
         ++row0_it)
    // for (auto row0_it: row0_range)
    {
      // fmt::print("Parent row node {}/{}\n", row0_it - row0_range.begin(),
      // row0_range.size());
      auto node_variant = make_node_view_variant(*row0_it);
      if (std::holds_alternative<EmptyNodeViewMut>(node_variant)) {
        // fmt::print("Parent node is an Empty node, nothing done\n");
        // There can be as many end of word nodes on the parent row as you like,
        // they do not affect the row below.
      } else if (auto* parent_node =
                     std::get_if<FullNodeViewMut>(&node_variant)) {

        // fmt::print("Parent node is a full node {}\n", *parent_node);
        // Here the parent node is a full node.
        // const auto data_size = parent_node->data_size();

        // First process and set the offset of the child from the row start and
        // set this on the parent
        // fmt::print("Setting next row offset to {}\n", offset);
        parent_node->set_next_row_offset(offset);

        assert(row1_it != row1_end);
        assert(*row1_it != data_.end());
        // Move past the first child node as this node's position is the offset
        // Add that child node's size to the offset
        // fmt::print("Child node: {}\n", node_to_string(*row1_it));
        // fmt::print("Finding node size to add\n");
        const auto sz = node_size(*row1_it);
        // fmt::print("node_size(*row1_it) {}\n", sz);

        // Each mini offset in the parent node is added to the node offset
        // so we minus the node offset
        const auto mini_offset_start = offset;
        offset += sz;

        // fmt::print("offset increased to {}\n", offset);

        // fmt::print("Child row node incrementing from {}/{}\n",
        // row1_it - row1_range.begin(), row1_range.size());
        ++row1_it;

        // Now for every mini offset in the parent, ie. for every child (n - 1
        // of them), go through child nodes and set mini offsets accordingly.

        auto mini_offsets = parent_node->mini_offsets();
        // fmt::print("mini_offsets size is {}\n", mini_offsets.size());
        // fmt::print("mini_offsets are {}\n", mini_offsets);

        if (mini_offsets.size() > 0) {
          assert(row1_it != row1_end);
          // fmt::print("Child node after ++: {}\n", node_to_string(*row1_it));
        }

        const auto last = mini_offsets.end();
        for (auto it = mini_offsets.begin(); it != last; ++it) {
          // fmt::print("mini_offsets iteration: Writing mini_offset {}\n",
          // offset);
          assert(row1_it != row1_end);
          // fmt::print("Child node is: {}\n", node_to_string(*row1_it));
          // fmt::print("Parent node before mini_offset written: {}\n",
          // *parent_node);
          assert(mini_offset_start <= offset);
          const auto mini_offset = offset - mini_offset_start;
          assert(mini_offset != 0);
          it.write(mini_offset);
          // fmt::print("Parent node after mini_offset written: {}\n",
          // *parent_node);
          assert(row1_it != row1_end);
          const auto szz = node_size(*row1_it);
          // fmt::print("Adding szz (node_size(*row1_it)) {} to offset\n", szz);
          offset += szz;
          // fmt::print("Child row node incrementing from {}/{}\n",
          // row1_it - row1_range.begin(), row1_range.size());
          ++row1_it;

          // fmt::print("---\n");
        }

      } else {
        // TODO: replace with proper assert lib UNREACHABLE
        assert(false && "UNREACHABLE");
      }
    }
  }
}

std::size_t CompactTrie2::size() const { return size_; }

std::size_t CompactTrie2::data_size() const { return data_.size(); }

bool CompactTrie2::empty() const { return this->size() == 0; }

template <class T, class Value>
static std::optional<std::size_t> at(const T& c, Value&& v) {
  for (auto&& [i, item] : ranges::views::enumerate(c)) {
    if (item == v)
      return i;
  }
  return {};
}

/*
 * Checks if a character c is present in the node pointed to by the iterator it.
 * If so, returns the offset of the corresponding node for c, in the next row.
 * Returns an empty optional if not.
 */
static std::optional<std::size_t> next_node_offset(const DataIterator it,
                                                   const char c) {
  auto node_variant = make_node_view_variant(it);
  if (std::holds_alternative<EmptyNodeView>(node_variant)) {
    return {};
  }

  FullNodeView node = std::get<FullNodeView>(node_variant);
  const std::optional<std::size_t> i = at(node.data(), c);
  if (!i) {
    return {};
  }

  // Position of node in the next row
  const std::size_t offset = node.next_row_offset();
  if (*i == 0) {
    return offset;
  }
  // *i > 0
  assert(*i > 0);

  // Position of the particular letter in the node on the next row
  const auto additional_offset = node.mini_offsets()[static_cast<long>(*i - 1)];
  return offset + additional_offset;
}

bool CompactTrie2::contains(const std::string_view word) const {
  if (this->empty())
    return false;
  auto [i, it, rows_it] = this->search(word, data_.begin(), rows_.begin());
  return i == word.size() && node_is_end_of_word(it);
}

bool CompactTrie2::further(const std::string_view word) const {
  if (this->empty())
    return false;
  auto [i, it, rows_it] = this->search(word, data_.begin(), rows_.begin());
  return i == word.size() && node_data_size(it) > 0;
}

// it -> data iterator
// rows_it -> the row of that iterator (will be ++ to get the next row start)
// i -> index, if i == word.size() found end

// template<class DataIterator, class RowIterator>
// bool
std::tuple<std::size_t, DataIterator, RowIterator>
CompactTrie2::search(const std::string_view word, DataIterator it,
                     RowIterator rows_it) const {
  assert(!this->empty());

  const bool use_cache = it == data_.begin();

  // fmt::print("\nSearching for word: {}\n", word);

  // auto it = data_.begin();
  // auto rows_it = rows_.begin();

  // FIXME: for now so don't forget about this problem/refactor to fix
  std::size_t i = 0;
  if (use_cache) {
    const auto* cached_result = cache_.lookup(word, i);
    if (cached_result) {
      std::tie(it, rows_it) = *cached_result;
    }
  }

  // for (; !word.empty(); word.remove_prefix(1))
  // std::size_t i = 0;
  for (; i < word.size();) {
    // const char c = word.front();
    const char c = word[i];
    // fmt::print("Searching for char: {} in {}\n", c, node_to_string(it));
    const std::optional<std::size_t> next_row_offset = next_node_offset(it, c);
    if (!next_row_offset) {
      // fmt::print("next_row_offset not found, returning false\n");
      return {i, it, rows_it};
      // return false;
      // return {it, rows_it, i, false};
    }

    // fmt::print("Incing \n");
    ++rows_it;
    // it = data_.begin() + static_cast<long>(*rows_it) +
    // static_cast<long>(*next_row_offset);
    it = *rows_it + static_cast<long>(*next_row_offset);
    ++i;
    if (use_cache)
      cache_.append(c, {it, rows_it});
  }

  assert(i == word.size());
  assert(it != data_.end());
  // const auto b = node_is_end_of_word(it);
  // fmt::print("Return {}\n", b);
  // fmt::print("Node on return {}\n", node_to_string(it));
  return {i, it, rows_it};
}

std::ostream& operator<<(std::ostream& os, const CompactTrie2& ct) {
  fmt::memory_buffer buff{};
  fmt::format_to(buff, "Trie of size: {} ({} bytes):\n", ct.size(),
                 ct.data_size());
  for (auto [i, row] : utility::make_adjacent_view(ct.rows_) |
                           ranges::views::transform([](const auto row) {
                             const auto [row_start, row_end] = row;
                             return ranges::subrange(row_start, row_end);
                           }) |
                           ranges::views::enumerate) {
    fmt::format_to(buff, "Row i: {}\n", i);
    for (auto it : NodeIteratorRange{ranges::views::all(row)}) {
      fmt::format_to(buff, "{}\n", node_to_string(it));
    }
  }
  return os << fmt::to_string(buff);
  // return os;
}

} // namespace compact_trie2
