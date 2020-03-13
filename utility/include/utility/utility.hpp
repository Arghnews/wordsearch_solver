#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/is_sorted.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/functional/identity.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range/primitives.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>

#include <string>
#include <tuple>
#include <string_view>
#include <type_traits>

namespace utility
{

// This rather hefty function produces a range of ranges from the input of a
// range of strings (or a container holding a container of chars).
// It returns a range of ranges that may be used as such:
// for (auto words_by_length: words_grouped_by_prefix_suffix(words))
//   for (auto [prefix, suffixes, is_end_of_word]: words_by_length)
template<class Rng>
auto words_grouped_by_prefix_suffix(Rng&& words_in)
{
  static_assert(ranges::forward_range<Rng>);
  auto words = ranges::views::all(words_in);
  assert(ranges::is_sorted(words));
  assert(!ranges::empty(words));
  const auto longest_word = (ranges::max(words |
      ranges::views::transform([] (const auto& word)
        {
          // decltype(word)::N;
          // return ranges::size(word);
          // return std::size(word);
        // fmt::print("On word: {}, ranges::size {}, word.size() {}\n",
            // word, ranges::size(word), word.size());
          return word.size();
        })));
  // fmt::print("longest_word: {}\n", longest_word);

  return ranges::views::ints(1UL, longest_word + 2)
    | ranges::views::transform(
      // This previously captured by reference
      // *Takes large breath*
      // FUUUU*****
      [words] (auto&& i)
      {
        // fmt::print("\nIteration, i: {}\n", i);

        // This says:
        // Filter (keepY) each word that has length >= i - 1
        // Transform into a prefix [0, i - 1) and a suffix at i - 1
        // Take unique
        // Then group by prefix
        return ranges::views::all(words)
          | ranges::views::filter(
              [i] (const auto& word)
              {
                return word.size() >= i - 1;
              })
          | ranges::views::transform(
              // Prefix, suffix (one char or empty string), bool represents if
              // this marks the end of a word or not.
              [i] (const auto& word) -> std::tuple<std::string_view, char, bool>
              {
                if (i - 1 == word.size())
                {
                  return {{word.data(), i - 1}, {}, true};
                }
                return {{word.data(), i - 1}, word[i - 1], false};
              })
          | ranges::views::unique
          | ranges::views::group_by(
              [] (const auto& tup0, const auto& tup1)
              {
                return std::get<0>(tup0) == std::get<0>(tup1);
              })
          ;
      })
    | ranges::views::transform( // For each prefix group
        ranges::views::transform( // For each tuple of that group
          [] (auto&& ww)
          {
            assert(!ranges::empty(ww));
            const auto prefix = std::get<0>(ww.front());
            static_assert(!std::is_reference_v<decltype(prefix)>,
                "This returns by value as transform constructs tuples. "
                "A reference will dangle.");

            // Unsure how to accomplish this in rangev3 style without two passes?
            // Lambda capture ref to bool end_of_word didn't work (assuming due to
            // rangev3 reordering freedoms, force eager evaluation of suffixes?)
            // Regardless leaving like this unless this is a performance problem
            //
            // Discard end of word elements and keep the suffixes
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

            // Having ranges::to<std::string> may alloc but helps to simplify
            // calling code by making the suffixes range sized and at least a
            // forward_range (as it's now a std::string)
            return std::tuple{prefix, suffixes | ranges::to<std::string>(),
              is_end_of_word};
          })
      )
  ;
}

}

#endif // UTILITY_HPP
