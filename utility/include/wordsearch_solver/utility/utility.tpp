#ifndef UTILITY_UTILITY_TPP
#define UTILITY_UTILITY_TPP

#include "fmt/ranges.h"
#include <fmt/core.h>
#include <fmt/format.h>

#include <range/v3/action/remove_if.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/is_sorted.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/functional/identity.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range/primitives.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>
#include <range/v3/view/zip.hpp>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace utility {

namespace detail {

template <class Rng>
auto words_grouped_by_prefix_suffix_impl(const Rng& words) {
  static_assert(ranges::forward_range<Rng>);
  auto words_view = ranges::views::all(words);
  assert(ranges::is_sorted(words_view));
  assert(!ranges::empty(words_view));
  const auto longest_word =
      (ranges::max(words_view | ranges::views::transform([](const auto& word) {
                     return word.size();
                   })));

  return ranges::views::ints(1UL, longest_word + 2) |
         ranges::views::transform(
             // This previously captured by reference
             // *Takes large breath*
             // FUUUU*****
             // Will dangle when returned as words view is on stack
             [words_view](auto&& i) {
               // This says:
               // Filter (keepY) each word that has length >= i - 1
               // Transform into a prefix [0, i - 1) and a suffix at i - 1
               // Take unique
               // Then group by prefix
               return ranges::views::all(words_view) |
                      ranges::views::filter([i](const auto& word) {
                        return word.size() >= i - 1;
                      }) |
                      ranges::views::transform(
                          // Prefix, suffix (one char or empty string), bool
                          // represents if this marks the end of a word or not.
                          [i](const auto& word)
                              -> std::tuple<std::string_view, char, bool> {
                            if (i - 1 == word.size()) {
                              return {{word.data(), i - 1}, {}, true};
                            }
                            return {{word.data(), i - 1}, word[i - 1], false};
                          }) |
                      ranges::views::unique |
                      ranges::views::group_by(
                          [](const auto& tup0, const auto& tup1) {
                            return std::get<0>(tup0) == std::get<0>(tup1);
                          });
             }) |
         ranges::views::transform(     // For each prefix group
             ranges::views::transform( // For each tuple of that group
                 [](auto&& ww) {
                   assert(!ranges::empty(ww));
                   const auto prefix = std::get<0>(ww.front());
                   static_assert(
                       !std::is_reference_v<decltype(prefix)>,
                       "This returns by value as transform constructs tuples. "
                       "A reference will dangle.");

                   // Unsure how to accomplish this in rangev3 style without two
                   // passes? Lambda capture ref to bool end_of_word didn't work
                   // (assuming due to rangev3 reordering freedoms, force eager
                   // evaluation of suffixes?) Regardless leaving like this
                   // unless this is a performance problem
                   //
                   // Discard end of word elements and keep the suffixes
                   auto suffixes =
                       ww |
                       ranges::views::remove_if(
                           [](const auto& prefix_suffix_endofword) {
                             return std::get<2>(prefix_suffix_endofword);
                           }) |
                       ranges::views::transform(
                           [](const auto& prefix_suffix_endofword) {
                             return std::get<1>(prefix_suffix_endofword);
                           });

                   const bool is_end_of_word = ranges::any_of(
                       ww | ranges::views::transform(
                                [](const auto& prefix_suffix_endofword) {
                                  return std::get<2>(prefix_suffix_endofword);
                                }),
                       ranges::identity());

                   // Having ranges::to<std::string> may alloc but helps to
                   // simplify calling code by making the suffixes range sized
                   // and at least a forward_range (as it's now a std::string)
                   return std::tuple{prefix,
                                     suffixes | ranges::to<std::string>(),
                                     is_end_of_word};
                 }));
}

} // namespace detail

template <class Rng> auto words_grouped_by_prefix_suffix(const Rng& words) {
  using ReturnType =
      decltype(detail::words_grouped_by_prefix_suffix_impl(words));
  // Use ranges::empty rather than words.empty to support things like
  // initializer_list
  if (ranges::empty(words)) {
    return ReturnType{};
  } else {
    if (!ranges::is_sorted(words)) {
      throw std::runtime_error("words_grouped_by_prefix_suffix given unsorted "
                               "range, must be sorted");
    }
    return detail::words_grouped_by_prefix_suffix_impl(words);
  }
}

template <class String> void throw_if_not_lowercase_ascii(const String& word) {
  // Certain dictionary data structures only accept lowercase ascii. Check a
  // word, and throw an exception if a character isn't lowercase ascii
  for (const auto c : word) {
    if (!(c >= 97 && c < 123)) {
      throw std::runtime_error(fmt::format(
          "Error - character: \"{}\" in word: \"{}\" not lowercase ascii", c,
          word));
    }
  }
}

/** Given [1, 2, 3, 4], returns [{1, 2}, {2, 3}, {3, 4}]
 * @param[in] rng A forward range or better
 */
template <class Range> auto make_adjacent_view(const Range& rng) {
  return ranges::views::zip(ranges::views::all(rng),
                            ranges::views::all(rng) | ranges::views::drop(1));
}

/** Outputs a row view onto @p data_view for each difference between indexes in
 * @p row_indexes
 *
 * @param data_view[in] A view onto data
 * @param row_indexes[in] Indexes splitting the data view into rows
 *
 * Example:
 * `make_row_view("abcdef", {0, 2, 5, 6}) => ("ab", "cde", "f")`
 */
template <class DataView, class RowIndexes>
auto make_row_view(DataView&& data_view, const RowIndexes& row_indexes) {
  return make_adjacent_view(row_indexes) |
         ranges::views::transform(
             [data_view = std::forward<DataView>(data_view)](const auto row) {
               const auto [row_start, row_end] = row;
               return ranges::subrange(row_start, row_end);
             });
}

} // namespace utility

#endif // UTILITY_UTILITY_TPP
