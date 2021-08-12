#ifndef UTILITY_UTILITY_HPP
#define UTILITY_UTILITY_HPP

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
#include <range/v3/view/filter.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace utility {

// This rather hefty function produces a range of ranges from the input of a
// range of strings (or a container holding a container of chars).
// It returns a range of ranges that may be used as such:
// for (auto words_by_length: words_grouped_by_prefix_suffix(words))
//   for (auto [prefix, suffixes, is_end_of_word]: words_by_length)
template <class Rng> auto words_grouped_by_prefix_suffix(Rng&& words_in) {
  static_assert(ranges::forward_range<Rng>);
  auto words = ranges::views::all(words_in);
  assert(ranges::is_sorted(words));
  assert(!ranges::empty(words));
  const auto longest_word =
      (ranges::max(words | ranges::views::transform(
                               [](const auto& word) { return word.size(); })));

  return ranges::views::ints(1UL, longest_word + 2) |
         ranges::views::transform(
             // This previously captured by reference
             // *Takes large breath*
             // FUUUU*****
             // Will dangle when returned as words view is on stack
             [words](auto&& i) {
               // This says:
               // Filter (keepY) each word that has length >= i - 1
               // Transform into a prefix [0, i - 1) and a suffix at i - 1
               // Take unique
               // Then group by prefix
               return ranges::views::all(words) |
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

inline void close_file(std::FILE* fp) {
  if (fp)
    std::fclose(fp);
}

// We use C style internals, fopen requires a null terminated string
// std::string_view does not guarantee null termination

namespace detail {
template <class ConsumeFile>
auto read_file_wrapper(const char* filepath, ConsumeFile&& f) {
  // https://en.cppreference.com/w/cpp/io/c/fgetc

  std::unique_ptr<std::FILE, decltype(&close_file)> fp(
      std::fopen(filepath, "r"), &close_file);
  if (!fp) {
    throw std::runtime_error(fmt::format("Cannot open file {}", filepath));
  }

  auto val = f(fp);

  if (std::ferror(fp.get())) {
    throw std::runtime_error("I/O error when reading");
  } else if (!std::feof(fp.get())) {
    throw std::runtime_error("End of file not reached");
  }

  return val;
}
} // namespace detail

inline std::vector<std::string> read_file_as_lines(const char* filepath) {
  auto ff = [](const auto& fp) {
    std::vector<std::string> lines;
    std::string line;

    int c; // note: int, not char, required to handle EOF
    while ((c = std::fgetc(fp.get())) != EOF) {
      if (c == '\n') {
        lines.push_back(std::move(line));
        line.clear();
      } else {
        line.push_back(static_cast<char>(c));
      }
    }

    return lines;
  };
  return detail::read_file_wrapper(filepath, ff);
}

inline std::vector<std::string>
read_file_as_lines(const std::string& filepath) {
  return read_file_as_lines(filepath.data());
}

inline std::string read_file_as_string(const char* filepath) {
  auto ff = [](const auto& fp) {
    std::string file_contents;
    int c; // note: int, not char, required to handle EOF
    while ((c = std::fgetc(fp.get())) != EOF) {
      file_contents.push_back(static_cast<char>(c));
    }
    return file_contents;
  };
  return detail::read_file_wrapper(filepath, ff);
}

inline std::string read_file_as_string(const std::string& filepath) {
  return read_file_as_string(filepath.data());
}

inline std::vector<std::string>
read_file_as_lines_keep_lowercase_ascii_only(const char* filepath) {
  return read_file_as_lines(filepath) |
         ranges::actions::remove_if([](const auto& word) {
           for (const auto c : word) {
             if (!(c >= 97 && c < 123)) {
               return true;
             }
           }
           return false;
         });
}

inline std::vector<std::string>
read_file_as_lines_keep_lowercase_ascii_only(const std::string& filepath) {
  return read_file_as_lines_keep_lowercase_ascii_only(filepath.data());
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

} // namespace utility

#endif // UTILITY_UTILITY_HPP
