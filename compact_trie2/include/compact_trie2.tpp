#ifndef COMPACT_TRIE2_TPP
#define COMPACT_TRIE2_TPP

// Here so that editor's language server is happy
#include "compact_trie2.hpp"

#include "empty_node_view.hpp"
#include "flat_char_value_map.hpp"
#include "full_node_view.hpp"

// TODO: chop down these headers to only what's needed, does clang-tidy do this
// automagically?
#include <range/v3/action/push_back.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/is_sorted.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/functional/identity.hpp>
#include <range/v3/iterator/access.hpp>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/iterator/operations.hpp>
#include <range/v3/iterator/traits.hpp>
#include <range/v3/range/access.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range/primitives.hpp>
#include <range/v3/view/adaptor.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/facade.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/slice.hpp>
// #include <range/v3/view/subrange.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>
#include <range/v3/view/zip.hpp>

#include <prettyprint.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <limits>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <variant>

// TODO: move header stuff into a header and rest into a .tcc

// using Container = std::vector<std::uint8_t>;
// using Iterator = Container::iterator;

// using Iter = ranges::iterator_t<Container>;
// static_assert(std::is_same_v<Iter, int>);

template<class Iterator>
using NodeVariant = std::variant<EmptyNodeView_<Iterator>,
      FullNodeView_<Iterator> >;

template<class Rng>
auto words_grouped_by_prefix_suffix(Rng&& words_in)
{
  static_assert(ranges::forward_range<Rng>);
  auto words = ranges::views::all(words_in);
  assert(ranges::is_sorted(words));
  const auto longest = (ranges::max(words |
      ranges::views::transform([] (const auto& word)
        {
          // decltype(word)::N;
          // return ranges::size(word);
          // return std::size(word);
        // fmt::print("On word: {}, ranges::size {}, word.size() {}\n",
            // word, ranges::size(word), word.size());
          return word.size();
        })));
  // fmt::print("longest: {}\n", longest);

  return ranges::views::ints(1UL, longest + 2)
    | ranges::views::transform(
      // This previously captured by reference
      // *Takes large breath*
      // FUUUU*****
      [words] (auto&& i)
      {
        // fmt::print("\nIteration, i: {}\n", i);

        auto w = ranges::views::all(words)
          | ranges::views::filter(
              [i] (const auto& word)
              {
                return word.size() + 1 >= i;
              })
          | ranges::views::transform(
              // Prefix, suffix (one char or empty string), bool represents if this
              // marks the end of a word or not.
              [i] (const auto& word) -> std::tuple<std::string_view, char, bool>
              {
                if (i == word.size() + 1)
                {
                  return {{word.data(), i - 1}, {}, true};
                }
                return {{word.data(), i - 1}, word.at(i - 1), false};
              })
          | ranges::views::unique
          | ranges::views::group_by(
              [] (const auto& tup0, const auto& tup1)
              {
                return std::get<0>(tup0) == std::get<0>(tup1);
              })
          ;

        return w;
      })
    | ranges::views::transform(
        ranges::views::transform(
          [] (auto&& ww)
          {
            const auto prefix = std::get<0>(ww.front());
            static_assert(!std::is_reference_v<decltype(prefix)>, "This returns by "
                "value as transform constructs tuples. A reference will dangle.");

            // Unsure how to accomplish this in rangev3 style without two passes?
            // Lambda capture ref to bool end_of_word didn't work (assuming due to
            // rangev3 reordering freedoms, force eager evaluation of suffixes?)
            // Regardless leaving like this unless this is a performance problem
            auto suffixes = ww
            | ranges::views::remove_if(
                [] (const auto& prefix_suffix_endofword)
                {
                  return std::get<2>(prefix_suffix_endofword);
                })
            | ranges::views::transform(
                [] (const auto& prefix_suffix_endofword)
                {
                  return std::get<1>(prefix_suffix_endofword);
                })
            ;

            const bool is_end_of_word = ranges::any_of(ww
                | ranges::views::transform(
                  [] (const auto& prefix_suffix_endofword)
                  {
                    return std::get<2>(prefix_suffix_endofword);
                  }),
                ranges::identity());

            return std::tuple{prefix, suffixes, is_end_of_word};
          })
      )
  ;
}

template<class Iterator>
NodeVariant<Iterator> make_node_view_variant(Iterator it)
{
  if (*it == 0)
  {
    return NodeVariant<Iterator>{
      std::in_place_type_t<EmptyNodeView_<Iterator>>{}, it};
  }
  return NodeVariant<Iterator>{
    std::in_place_type_t<FullNodeView_<Iterator>>{}, it};
}

template<class Iterator>
auto node_size(Iterator it)
{
  return std::visit([] (auto&& node) { return node.size(); },
      make_node_view_variant(it));
}

template<class Iterator>
auto node_data_size(Iterator it)
{
  return std::visit([] (auto&& node) { return node.data_size(); },
      make_node_view_variant(it));
}

template<class Iterator>
bool node_is_end_of_word(Iterator it)
{
  return std::visit([] (auto&& node) { return node.is_end_of_word(); },
      make_node_view_variant(it));
}

template<class Iterator>
std::string node_to_string(Iterator it)
{
  return std::visit([] (auto&& node)
      {
        return fmt::format("{}", node);
      },
      make_node_view_variant(it));
}

// Ideally this function's range requirements would be significantly relaxed.
// (InputRange and ForwardOutputIterator)
// However it's hard to see exactly how to do this. Leaving for now
template<class OutputIterator, class ForwardCharsRange>
void make_node(ForwardCharsRange&& rng, OutputIterator out, bool is_end_of_word)
{
  static_assert(ranges::sized_range<ForwardCharsRange>,
      "The range passed to make node must be sized.");
  // static_assert(ranges::output_iterator<OutputIterator, std::uint8_t>);
  using SizeType = std::uint8_t;
  if (rng.size() == 0)
  {
    assert(is_end_of_word && "0 characters must mean it's a word end");
  }
  assert(rng.size() < std::numeric_limits<SizeType>::max());
  const auto size = static_cast<std::uint8_t>(rng.size());

  *out++ = size;

  if (size == 0)
  {
    return;
  }

  *out++ = 0;
  *out++ = 0;
  // *out++ = 0x80UL;
  *out++ = 0x80UL & (static_cast<unsigned long>(is_end_of_word) << 7UL);

  ranges::for_each(rng,
      [out] (const auto c) mutable
      {
        // 1 byte per letter/data item
        *out++ = static_cast<std::uint8_t>(c);
      });
  ranges::for_each(rng | ranges::views::drop_exactly(1),
      [out] (auto&&) mutable
      {
        // 2 bytes for each mini offset entry
        *out++ = 255;
        *out++ = 255;
      });
}

template<class Range>
auto make_adjacent_view(Range&& rng)
{
  return ranges::views::zip(
      ranges::views::all(std::forward<Range>(rng)),
      ranges::views::all(std::forward<Range>(rng))
        | ranges::views::drop(1));
}

template<class DataView, class RowIndexes>
auto make_row_view(DataView&& data_view, RowIndexes&& row_indexes)
{
  return make_adjacent_view(row_indexes)
    | ranges::views::transform(
      [data_view = std::forward<DataView>(data_view)] (const auto row)
      {
        const auto [row_start, row_end] = row;
        return ranges::views::slice(data_view, static_cast<long>(row_start),
            static_cast<long>(row_end));
      });
}

template<class DataView, class RowIndexes>
auto make_pairwise_rows_view(DataView&& data_view, RowIndexes&& row_indexes)
{
  return ranges::views::zip(
      make_row_view(data_view, row_indexes),
      make_row_view(data_view, row_indexes) | ranges::views::drop(1));
}


template<class T>
constexpr bool is_sortable(T&&)
{
  return std::is_invocable_v<decltype(ranges::sort), T>;
}

template<class T>
constexpr auto sort_if_possible_impl(T&& t, int)
  -> decltype(ranges::sort(t), void())
{
  ranges::sort(t);
}

template<class T>
constexpr void sort_if_possible_impl(T&&, long)
{}

template<class T>
constexpr void sort_if_possible(T&& t)
{
  sort_if_possible_impl(std::forward<T>(t), 0);
}

// Currently gcc8 complains with "-fconcepts" about incomplete type with
// std::tuple_element if I instead pass ranges::subrange(first, last).
// Not sure exactly why this is, but this is my temporary fix for now
template<class Iterator1, class Iterator2>
CompactTrie2::CompactTrie2(Iterator1 first, const Iterator2 last)
: CompactTrie2(std::vector<std::string>(first, last))
{}

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
template<class ForwardRange>
CompactTrie2::CompactTrie2(ForwardRange&& words)
: data_()
, rows_{0}
, size_(0)
, cache_()
{

  // Just not going to handle these. Need deep not pointer comparator and call
  // strlen to get size etc
  using Underlying = std::decay_t<ranges::range_value_t<decltype(words)>>;
  static_assert(!std::is_same_v<Underlying, const char*>, "Don't enter const "
      "char*s, convert to std::string_view or std::string.");

  if (!ranges::is_sorted(words))
  {
    // fmt::print("The range is not sorted\n");
    if (is_sortable(words))
    {
      // fmt::print("It's sortable\n");
      sort_if_possible(words);
      init(words);
    }
    else
    {
      // fmt::print("Not sortable, to vector\n");
      std::vector<std::string_view> strings;
      init(ranges::views::all(strings)
          | ranges::actions::push_back(words)
          | ranges::actions::sort
          | ranges::actions::unique);
    }
  }
  else
  {
    // fmt::print("The range is sorted, done\n");
    init(words);
  }
}

template<class T>
void CompactTrie2::init(T&& words_view)
{

  auto data_insert_iter = std::back_inserter(data_);

  for (auto words_by_length: words_grouped_by_prefix_suffix(words_view))
  {
    // fmt::print("\nIteration\n");
    // fmt::print("data: {}\n", data);
    // const auto old_row_end = static_cast<long>(data.size());
    for (auto [prefix, suffixes, is_end_of_word]: words_by_length)
    {
      size_ += is_end_of_word;
      // TODO: see if this allocation of a vector is a significant
      // performance penalty. If so, consider a llvm/boost etc small_vector
      make_node(
          ranges::views::all(suffixes) | ranges::to<std::vector>(),
          data_insert_iter, is_end_of_word);
      // fmt::print("{} -> {}, end_of_word: {}\n", prefix,
         // suffixes | ranges::to<std::vector>(), is_end_of_word);
    }
    const auto new_row_end = static_cast<long>(data_.size());
    // fmt::print("row {}\n", new_row_end);
    // fmt::print("row {} -> {}\n", old_row_end, new_row_end);
    ranges::push_back(rows_, new_row_end);

  }

  non_templated_rest_of_init();
}

template<class OutputIndexIterator>
void CompactTrie2::contains_and_further(const std::string& stem,
    const std::string& suffixes,
    OutputIndexIterator contains_out_it,
    OutputIndexIterator further_out_it,
    OutputIndexIterator contains_and_further_out_it) const
{
  if (this->empty()) return;
  const auto [stem_index, it, rows_it] = this->search(stem, data_.begin(),
      rows_.begin());
  if (stem_index < stem.size()) return;

  for (const auto [i, c]: suffixes | ranges::views::enumerate)
  {
    const std::string_view suffix{&c, 1};
    const auto [suffix_i, suffix_it, suffix_rows_it] = this->search(suffix,
        it, rows_it);

    // const bool contains = false;
    const bool contains = suffix.size() == suffix_i && node_is_end_of_word(
        suffix_it);
    // const bool further = false;
    const bool further = suffix.size() == suffix_i && node_data_size(
        suffix_it) > 0;

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

#endif
