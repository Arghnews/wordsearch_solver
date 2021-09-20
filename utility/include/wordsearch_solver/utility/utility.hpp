#ifndef UTILITY_UTILITY_HPP
#define UTILITY_UTILITY_HPP

#include <string>
#include <vector>

/** Utility functions
 */
namespace utility {

/** This rather hefty function produces a range of ranges from the input of a
 * range of strings (or a container holding a container of chars).
 *
 * @param[in] words A lexicographically sorted container or view of strings
 * @returns See example below.
 *
 * @note The input range to this function MUST be sorted, else throws.
 * @throws std::runtime_error If the input range is not sorted
 *
 * It is used for construction of tries.
 * It returns a range of ranges that may be used as such:
 * ```cpp
 * std::vector<std::string> lines = {
 *     "woods", "woody", "wood", "a", "ask",
 * };
 * std::sort(lines.begin(), lines.end());
 * for (auto words_by_length : utility::words_grouped_by_prefix_suffix(lines)) {
 *   for (auto [prefix, suffix, end_of_word] : words_by_length) {
 *     fmt::print("Word: \"{}\", suffixes: [{}], end_of_word: {}\n", prefix,
 *                suffix, end_of_word);
 *   }
 *   fmt::print("\n");
 * }
 * ```
 * ```
 * Output:
 * Word: "", suffixes: [aw], end_of_word: false
 *
 * Word: "a", suffixes: [s], end_of_word: true
 * Word: "w", suffixes: [o], end_of_word: false
 *
 * Word: "as", suffixes: [k], end_of_word: false
 * Word: "wo", suffixes: [o], end_of_word: false
 *
 * Word: "ask", suffixes: [], end_of_word: true
 * Word: "woo", suffixes: [d], end_of_word: false
 *
 * Word: "wood", suffixes: [sy], end_of_word: true
 *
 * Word: "woods", suffixes: [], end_of_word: true
 * Word: "woody", suffixes: [], end_of_word: true
 * ```
 */
template <class Rng> auto words_grouped_by_prefix_suffix(const Rng& words);

/** Read a file at path @p filepath and return it split by newlines into a
 * `std::vector<std::string>`
 *
 * @param[in] filepath
 * @returns `std::vector<std::string>` Each line as a string
 *
 * @throws std::runtime_error If there is an error with file opening (file
 * doesn't exist, etc)
 */
std::vector<std::string> read_file_as_lines(const char* filepath);
/** @overload */
std::vector<std::string> read_file_as_lines(const std::string& filepath);

/** Read a file at path @p filepath into a `std::string`
 *
 * @param[in] filepath
 * @returns `std::string` The file in a string
 *
 * @throws std::runtime_error If there is an error with file opening (file
 * doesn't exist, etc)
 */
std::string read_file_as_string(const char* filepath);
/** @overload */
std::string read_file_as_string(const std::string& filepath);

/** Just like read_file_as_lines() except only keeps lines where all characters
 * are lowercase ascii
 *
 * @copydoc read_file_as_lines()
 */
std::vector<std::string>
read_file_as_lines_keep_lowercase_ascii_only(const char* filepath);
/** @overload */
std::vector<std::string>
read_file_as_lines_keep_lowercase_ascii_only(const std::string& filepath);

/**
 * @throws std::runtime_error
 */
template <class String> void throw_if_not_lowercase_ascii(const String& word);

/** Given a range `[1, 2, 3, 4]`, returns `[{1, 2}, {2, 3}, {3, 4}]`
 * @param[in] rng A forward range or better
 */
template <class Range> auto make_adjacent_view(const Range& rng);

/** Outputs a row view onto @p data_view for each difference between indexes in
 * @p row_indexes
 *
 * @param[in] data_view A view onto data
 * @param[in] row_indexes Indexes splitting the data view into rows
 *
 * Example:
 * `make_row_view("abcdef", {0, 2, 5, 6}) => ("ab", "cde", "f")`
 */
template <class DataView, class RowIndexes>
auto make_row_view(DataView&& data_view, const RowIndexes& row_indexes);

} // namespace utility

#include "wordsearch_solver/utility/utility.tpp"

#endif // UTILITY_UTILITY_HPP
