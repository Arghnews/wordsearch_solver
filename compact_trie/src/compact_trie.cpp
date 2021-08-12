#include "wordsearch_solver/compact_trie/compact_trie.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <range/v3/view/all.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <string_view>
#include <utility>

// This might ACTUALLY be a case for inheritance what with the CompactTrie being
// a Trie?
// TODO: test this with const_iterator not a std::tuple, and try a simple user
// defined struct/pointer to make trivial type to help the optimiser? Not even
// sure if "trivial" means what I think it does anyway, remove this likely..
// TODO: maybe look into units library for the ascii/index conversion stuff, as
// that has already wasted a significant amount of time with offset stuff

namespace {

template <class Range> auto make_adjacent_view(Range&& rng) {
  return ranges::views::zip(ranges::views::all(std::forward<Range>(rng)),
                            ranges::views::all(std::forward<Range>(rng)) |
                                ranges::views::drop(1));
}

template <class DataView, class RowIndexes>
auto make_row_view(DataView&& data_view, RowIndexes&& row_indexes) {
  return make_adjacent_view(row_indexes) |
         ranges::views::transform(
             [data_view = std::forward<DataView>(data_view)](const auto row) {
               const auto [row_start, row_end] = row;
               return ranges::subrange(row_start, row_end);
               // return ranges::views::slice(data_view,
               // static_cast<long>(row_start), static_cast<long>(row_end));
             });
}

} // namespace

namespace compact_trie {

CompactTrie::CompactTrie(const std::initializer_list<std::string_view>& words)
    : CompactTrie(ranges::views::all(words)) {}

CompactTrie::CompactTrie(const std::initializer_list<std::string>& words)
    : CompactTrie(ranges::views::all(words)) {}

CompactTrie::CompactTrie(const std::initializer_list<const char*>& words)
    : CompactTrie(ranges::views::all(words) |
                  ranges::views::transform([](const auto string_literal) {
                    return std::string_view{string_literal};
                  })) {}

bool CompactTrie::contains(const std::string_view word,
                           const ranges::subrange<NodesIterator> nodes,
                           const ranges::subrange<RowsIterator> rows) const {
  const auto [node_it, rows_it] = this->search(word, nodes, rows);
  if (node_it == nodes.end()) {
    return false;
  }
  const auto letters_consumed =
      static_cast<std::size_t>(std::distance(rows.begin(), rows_it));
  return letters_consumed == word.size() && node_it->is_end_of_word();
}

bool CompactTrie::contains(const std::string_view word) const {
  return contains(word, ranges::subrange(nodes_), ranges::subrange(rows_));
}

bool CompactTrie::further(const std::string_view word,
                          const ranges::subrange<NodesIterator> nodes,
                          const ranges::subrange<RowsIterator> rows) const {
  const auto [node_it, rows_it] = this->search(word, nodes, rows);
  if (node_it == nodes.end()) {
    return false;
  }
  const auto letters_consumed =
      static_cast<std::size_t>(std::distance(rows.begin(), rows_it));
  return letters_consumed == word.size() && node_it->any();
}

bool CompactTrie::further(const std::string_view word) const {
  return further(word, ranges::subrange(nodes_), ranges::subrange(rows_));
}

std::size_t CompactTrie::size() const { return size_; }

bool CompactTrie::empty() const { return size_ == 0; }

std::ostream& operator<<(std::ostream& os, const CompactTrie& ct) {
  fmt::memory_buffer buff{};
  fmt::format_to(buff, "Size: {}\n", ct.size());
  for (const auto [i, row] :
       ranges::views::enumerate(make_row_view(ct.nodes_, ct.rows_))) {
    fmt::format_to(buff, "Row: {}\n", i);
    for (const auto& node : row) {
      fmt::format_to(buff, "{}", node);
    }
    fmt::format_to(buff, "\n");
  }
  fmt::format_to(buff, "Rows: {}\n",
                 ranges::views::all(ct.rows_) |
                     ranges::views::transform([&ct](const auto row_it) {
                       return std::distance(ct.nodes_.begin(), row_it);
                     }));
  return os << fmt::to_string(buff);
}

namespace {

CompactTrie::NodesIterator follow(const CompactTrie::NodesIterator node_it,
                                  CompactTrie::RowsIterator rows_it,
                                  const std::uint8_t index,
                                  const CompactTrie::NodesIterator nodes_end,
                                  const CompactTrie::RowsIterator rows_end) {
  assert(node_it != nodes_end);
  assert(rows_it != rows_end);

  if (!node_it->test(index) || ++rows_it == rows_end) {
    return nodes_end;
  }

  const auto before_node = node_it->preceding();
  const auto before_in_node = node_it->bits_on_before(index);
  const auto preceding = before_node + before_in_node;

  return *rows_it + static_cast<long>(preceding);
}

} // namespace

CompactTrie::const_iterator CompactTrie::search(
    std::string_view word,
    const ranges::subrange<CompactTrie::NodesIterator> nodes,
    const ranges::subrange<CompactTrie::RowsIterator> rows) const {
  if (nodes.empty()) {
    return {nodes.end(), rows.end()};
  }

  auto it = nodes.begin();
  auto rows_it = rows.begin();

  // const bool use_cache = nodes.begin() == nodes_.begin();
  // if (use_cache)
  // {
  // if (const auto cached_result = cache_.lookup(word))
  // {
  // const auto [elems_consumed, value] = *cached_result;
  // word.remove_prefix(elems_consumed);
  // std::tie(it, rows_it) = value;
  // }
  // }

  for (const auto c : word) {
    // fmt::print("\nFollowing: {}\n", c);

    const auto next_it = follow(it, rows_it, static_cast<std::uint8_t>(c),
                                nodes.end(), rows.end());
    if (next_it == nodes.end()) {
      assert(it != nodes.end());
      assert(rows_it != rows.end());
      return {it, rows_it};
    }
    // fmt::print("Node now: {}, row: {}, row_index: {}\n", *next_it,
    // rows_it - rows.begin(), it - nodes.begin() + *rows_it);
    it = next_it;
    ++rows_it;
    // if (use_cache)
    // {
    // cache_.append(c, {it, rows_it});
    // }
  }
  assert(it != nodes.end());
  assert(rows_it != rows.end());
  return {it, rows_it};
}

} // namespace compact_trie
