#ifndef COMPACT_TRIE2_TPP
#define COMPACT_TRIE2_TPP

// Here so that editor's language server is happy
#include "wordsearch_solver/compact_trie2/compact_trie2.hpp"

#include "wordsearch_solver/compact_trie2/empty_node_view.hpp"
#include "wordsearch_solver/compact_trie2/full_node_view.hpp"
#include "wordsearch_solver/utility/flat_char_value_map.hpp"
#include "wordsearch_solver/utility/utility.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <range/v3/action/push_back.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/is_sorted.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>
#include <range/v3/view/zip.hpp>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// TODO: move header stuff into a header and rest into a .tcc

// using Container = std::vector<std::uint8_t>;
// using Iterator = Container::iterator;

// using Iter = ranges::iterator_t<Container>;
// static_assert(std::is_same_v<Iter, int>);

namespace compact_trie2 {

// TODO: try without std::variant and use own class (should be relatively easy
// to implement), see performance difference
template <class Iterator>
using NodeVariant =
    std::variant<EmptyNodeView_<Iterator>, FullNodeView_<Iterator>>;

template <class Iterator>
NodeVariant<Iterator> make_node_view_variant(Iterator it) {
  if (*it == 0) {
    return NodeVariant<Iterator>{
        std::in_place_type_t<EmptyNodeView_<Iterator>>{}, it};
  }
  return NodeVariant<Iterator>{std::in_place_type_t<FullNodeView_<Iterator>>{},
                               it};
}

template <class Iterator> auto node_size(Iterator it) {
  return std::visit([](auto&& node) { return node.size(); },
                    make_node_view_variant(it));
}

template <class Iterator> auto node_data_size(Iterator it) {
  return std::visit([](auto&& node) { return node.data_size(); },
                    make_node_view_variant(it));
}

template <class Iterator> bool node_is_end_of_word(Iterator it) {
  return std::visit([](auto&& node) { return node.is_end_of_word(); },
                    make_node_view_variant(it));
}

template <class Iterator> std::string node_to_string(Iterator it) {
  return std::visit([](auto&& node) { return fmt::format("{}", node); },
                    make_node_view_variant(it));
}

/** Builds a node from a range of letters/suffixes.
 *
 * @param[in] suffixes The letters to insert into this node
 * @param[in] is_end_of_word Whether or not a word terminates here
 * @param[out] out The output iterator that the node is written out to
 */
template <class OutputIterator, class ForwardCharsRange>
void make_node(const ForwardCharsRange& suffixes, const bool is_end_of_word,
               OutputIterator out) {
  static_assert(ranges::sized_range<ForwardCharsRange>,
                "The range passed to make node must be sized.");
  // static_assert(ranges::output_iterator<OutputIterator, std::uint8_t>);
  if (suffixes.size() == 0) {
    assert(is_end_of_word && "0 characters must mean it's a word end");
  }
  assert(suffixes.size() < std::numeric_limits<std::uint8_t>::max());
  const auto size = static_cast<std::uint8_t>(suffixes.size());

  *out++ = size;

  if (size == 0) {
    return;
  }

  *out++ = 0;
  *out++ = 0;
  // *out++ = 0x80UL;
  *out++ = 0x80UL & (static_cast<unsigned long>(is_end_of_word) << 7UL);

  ranges::for_each(suffixes, [out](const auto c) mutable {
    // 1 byte per letter/data item
    *out++ = static_cast<std::uint8_t>(c);
  });
  ranges::for_each(suffixes | ranges::views::drop_exactly(1),
                   [out](auto&&) mutable {
                     // 2 bytes for each mini offset entry
                     *out++ = 255;
                     *out++ = 255;
                   });
}

/** Used in construction, returns a view onto pairs of {row n, row n + 1}
 */
template <class DataView, class RowIndexes>
auto make_adjacent_pairwise_rows_view(const DataView& data_view,
                                      const RowIndexes& row_indexes) {
  return ranges::views::zip(utility::make_row_view(data_view, row_indexes),
                            utility::make_row_view(data_view, row_indexes) |
                                ranges::views::drop(1));
}

template <class T> constexpr bool is_sortable(T&&) {
  return std::is_invocable_v<decltype(ranges::sort), T>;
}

template <class T>
constexpr auto sort_if_possible_impl(T&& t, int)
    -> decltype(ranges::sort(t), void()) {
  ranges::sort(t);
}

template <class T> constexpr void sort_if_possible_impl(T&&, long) {}

/** Sort if possible, but won't be a compilation error if not, then do nothing.
 *
 * @param[in] t Sort @p t if it may be sorted (ie. it's not const, it's a
 * mutable container not a view)
 */
template <class T> constexpr void sort_if_possible(T&& t) {
  sort_if_possible_impl(std::forward<T>(t), 0);
}

// Currently gcc8 complains with "-fconcepts" about incomplete type with
// std::tuple_element if I instead pass ranges::subrange(first, last).
// Not sure exactly why this is, but this is my temporary fix for now
template <class Iterator1, class Iterator2>
CompactTrie2::CompactTrie2(Iterator1 first, const Iterator2 last)
    : CompactTrie2(std::vector<std::string>(first, last)) {}

// Member functions in this class that could be const aren't because the
// EmptyNodeView and FullNodeView classes wrap iterators. These iterators are
// into data_'s type (std::vector<std::uint8_t>). However FullNodeView
// requires a mutable iterator (at least whilst building the data structure)
// as it mutates underlying data. But then if you try to call this->search()
// where search is marked as a const member, you can't because trying to make
// a std::variant<EmptyNodeView, FullNodeView> where each holds a (mutable)
// std::vector<std::uint8_t>::iterator won't work as you are in a const member
// function and therefore pass them std::vector<std::uint8_t>::const_iterator.

// TODO: awful SFINAE or wait until 2030 for widespread cpp20 concepts to
// constrain this to a ForwardRange
template <class ForwardRange>
CompactTrie2::CompactTrie2(ForwardRange&& words)
    : data_(), rows_{}, size_(0), cache_() {

  // Just not going to handle these. Need deep not pointer comparator and call
  // strlen to get size etc
  using Underlying = std::decay_t<ranges::range_value_t<decltype(words)>>;
  static_assert(!std::is_same_v<Underlying, const char*>,
                "Don't enter const "
                "char*s, convert to std::string_view or std::string.");

  if (!ranges::is_sorted(words)) {
    // fmt::print("The range is not sorted\n");
    if (is_sortable(words)) {
      // fmt::print("It's sortable\n");

      sort_if_possible(words);
      init(words);
    } else {
      // fmt::print("Not sortable, to vector\n");
      std::vector<std::string_view> strings;
      init(ranges::views::all(strings) | ranges::actions::push_back(words) |
           ranges::actions::sort | ranges::actions::unique);
    }
  } else {
    // fmt::print("The range is sorted, done\n");
    init(words);
  }
}

template <class T> void CompactTrie2::init(const T& words_view) {

  for (const auto& word : words_view) {
    utility::throw_if_not_lowercase_ascii(word);
  }

  auto data_insert_iter = std::back_inserter(data_);
  std::vector<std::size_t> rows{0};

  for (auto words_by_length :
       utility::words_grouped_by_prefix_suffix(words_view)) {
    // fmt::print("\nIteration\n");
    // fmt::print("data: {}\n", data);
    // const auto old_row_end = static_cast<long>(data.size());
    for (auto [prefix, suffixes, is_end_of_word] : words_by_length) {
      size_ += is_end_of_word;
      make_node(suffixes, is_end_of_word, data_insert_iter);
      // fmt::print("{} -> {}, end_of_word: {}\n", prefix,
      // suffixes | ranges::to<std::vector>(), is_end_of_word);
    }
    const auto new_row_end = data_.size();
    // fmt::print("row {}\n", new_row_end);
    // fmt::print("row {} -> {}\n", old_row_end, new_row_end);
    rows.push_back(new_row_end);
  }

  // Convert vector of indexes into data_, which is fine even if data_
  // reallocates, into a vector of iterators, as we're done mutating data_
  for (const auto& data_index : rows) {
    assert(data_index <= data_.size());
    rows_.push_back(std::next(data_.begin(), static_cast<long>(data_index)));
  }

  non_templated_rest_of_init();
}

template <class OutputIterator>
void CompactTrie2::contains_further(const std::string_view stem,
                                    const std::string_view suffixes,
                                    OutputIterator contains_further_it) const {
  if (this->empty())
    return;
  const auto [stem_index, it, rows_it] =
      this->search(stem, data_.begin(), rows_.begin());
  if (stem_index < stem.size())
    return;

  for (const auto [i, c] : suffixes | ranges::views::enumerate) {
    const std::string_view suffix{&c, 1};
    const auto [suffix_i, suffix_it, suffix_rows_it] =
        this->search(suffix, it, rows_it);

    // const bool contains = false;
    const bool contains =
        suffix.size() == suffix_i && node_is_end_of_word(suffix_it);
    // const bool further = false;
    const bool further =
        suffix.size() == suffix_i && node_data_size(suffix_it) > 0;
    *contains_further_it++ = {contains, further};
  }
}

} // namespace compact_trie2

#endif
