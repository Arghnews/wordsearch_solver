#ifndef UTILITY_UTILITY_HPP
#define UTILITY_UTILITY_HPP

#include <string>
#include <vector>

namespace utility {

// The input range to this function MUST be sorted, else throws.
// This rather hefty function produces a range of ranges from the input of a
// range of strings (or a container holding a container of chars).
// It is used for construction of tries.
// It returns a range of ranges that may be used as such:
// std::vector\<std::string\> lines = {
//     "woods", "woody", "wood", "a", "ask",
// };
// std::sort(lines.begin(), lines.end());
// for (auto words_by_length : utility::words_grouped_by_prefix_suffix(lines)) {
//   for (auto [prefix, suffix, end_of_word] : words_by_length) {
//     fmt::print("Word: \"{}\", suffixes: [{}], end_of_word: {}\n", prefix,
//                suffix, end_of_word);
//   }
//   fmt::print("\n");
// }
//
// Output:
// Word: "", suffixes: [aw], end_of_word: false
//
// Word: "a", suffixes: [s], end_of_word: true
// Word: "w", suffixes: [o], end_of_word: false
//
// Word: "as", suffixes: [k], end_of_word: false
// Word: "wo", suffixes: [o], end_of_word: false
//
// Word: "ask", suffixes: [], end_of_word: true
// Word: "woo", suffixes: [d], end_of_word: false
//
// Word: "wood", suffixes: [sy], end_of_word: true
//
// Word: "woods", suffixes: [], end_of_word: true
// Word: "woody", suffixes: [], end_of_word: true
template <class Rng> auto words_grouped_by_prefix_suffix(const Rng& words);

std::vector<std::string> read_file_as_lines(const char* filepath);
std::vector<std::string> read_file_as_lines(const std::string& filepath);

std::string read_file_as_string(const char* filepath);
std::string read_file_as_string(const std::string& filepath);

std::vector<std::string>
read_file_as_lines_keep_lowercase_ascii_only(const char* filepath);
std::vector<std::string>
read_file_as_lines_keep_lowercase_ascii_only(const std::string& filepath);

template <class String> void throw_if_not_lowercase_ascii(const String& word);

template <class Range> auto make_adjacent_view(const Range& rng);

template <class DataView, class RowIndexes>
auto make_row_view(DataView&& data_view, const RowIndexes& row_indexes);

} // namespace utility

#include "wordsearch_solver/utility/utility.tpp"

#endif // UTILITY_UTILITY_HPP
