#include "wordsearch_solver/trie/trie.hpp"
#include "wordsearch_solver/trie/node.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <range/v3/view/all.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/zip.hpp>

#include <cstddef>
#include <iterator>
#include <initializer_list>
#include <string_view>
#include <utility>
#include <vector>

// This might ACTUALLY be a case for inheritance what with the Trie being
// a Trie?
// TODO: test this with const_iterator not a std::tuple, and try a simple user
// defined struct/pointer to make trivial type to help the optimiser? Not even
// sure if "trivial" means what I think it does anyway, remove this likely..
// TODO: maybe look into units library for the ascii/index conversion stuff, as
// that has already wasted a significant amount of time with offset stuff

namespace
{

}

namespace trie
{

Trie::Trie(const std::initializer_list<std::string_view>& words)
  : Trie(ranges::views::all(words))
{}

Trie::Trie(const std::initializer_list<std::string>& words)
  : Trie(ranges::views::all(words))
{}

Trie::Trie(const std::initializer_list<const char*>& words)
  : Trie(ranges::views::all(words)
      | ranges::views::transform(
        [] (const auto string_literal)
        {
        return std::string_view{string_literal};
        }
        ))
{}

bool Trie::contains(const std::string_view word) const
{
  return detail::contains(root_, word);
}

bool Trie::further(const std::string_view word) const
{
  return detail::further(root_, word);
}

std::size_t Trie::size() const
{
  return size_;
}

bool Trie::empty() const
{
  return size_ == 0;
}

std::ostream& operator<<(std::ostream& os, const Trie& ct)
{
  fmt::memory_buffer buff{};
  fmt::format_to(buff, "Size: {}\n", ct.size());
  std::vector<const Node*> nodes{&ct.root_};
  std::vector<const Node*> next_row;
  while (!nodes.empty())
  {
    for (const auto* node: nodes)
    {
      fmt::format_to(buff, "{}", *node);
      for (const auto& edge: node->edges())
      {
        next_row.push_back(edge.child.get());
      }
    }
    fmt::format_to(buff, "\n");
    nodes = std::move(next_row);
  }
  return os << fmt::to_string(buff);
}

const Node* Trie::search(std::string_view word) const
{
  const Node* p = &root_;

  const bool use_cache = true;
  std::size_t i = 0;
  if (use_cache)
  {
    const auto* cached_result = cache_.lookup(word, i);
    if (cached_result)
    {
      word.remove_prefix(i);
      p = *cached_result;
    }
  }

  for (; !word.empty(); word.remove_prefix(1))
  {
    // fmt::print("p: {}\n", *p);
    const Node* next = p->test(word.front());
    if (!next)
    {
      // fmt::print("next is nullptr, ret\n");
      return nullptr;
    }
    // fmt::print("next: {}\n", *next);
    p = next;
    if (use_cache) cache_.append(word.front(), p);
  }
  return p;
}

std::pair<Node*, bool> Trie::insert(std::string_view word)
{
  Node* p = &root_;

  for (; !word.empty(); word.remove_prefix(1))
  {
    p = p->add_char(word.front());
  }
  if (!p->is_end_of_word())
  {
    p->set_is_end_of_word(true);
    return {p, true};
  }
  return {p, false};
}

namespace detail
{

bool contains(const Node& node, const std::string_view word)
{
  const auto* p = search(node, word);
  return p && p->is_end_of_word();
}

bool further(const Node& node, const std::string_view word)
{
  const auto* p = search(node, word);
  return p && p->any();
}

// Trie::NodesIterator follow(const Trie::NodesIterator node_it,
    // Trie::RowsIterator rows_it,
    // const std::uint8_t index,
    // const Trie::NodesIterator nodes_end,
    // const Trie::RowsIterator rows_end)
// {
  // assert(node_it != nodes_end);
  // assert(rows_it != rows_end);

  // if (!node_it->test(index) || ++rows_it == rows_end)
  // {
    // return nodes_end;
  // }

  // const auto before_node = node_it->preceding();
  // const auto before_in_node = node_it->bits_on_before(index);
  // const auto preceding = before_node + before_in_node;

  // return *rows_it + static_cast<long>(preceding);
// }

// Trie::const_iterator search(const std::string_view word,
    // const ranges::subrange<Trie::NodesIterator> nodes,
    // const ranges::subrange<Trie::RowsIterator> rows)
// {
  // if (nodes.empty())
  // {
    // return {nodes.end(), rows.end()};
  // }

  // auto it = nodes.begin();
  // auto rows_it = rows.begin();

  // // const bool use_cache = nodes.begin() == nodes_.begin();
  // // if (use_cache)
  // // {
    // // if (const auto cached_result = cache_.lookup(word))
    // // {
      // // const auto [elems_consumed, value] = *cached_result;
      // // word.remove_prefix(elems_consumed);
      // // std::tie(it, rows_it) = value;
    // // }
  // // }

  // for (const auto c: word)
  // {
    // // fmt::print("\nFollowing: {}\n", c);

    // const auto next_it = follow(it, rows_it, static_cast<std::uint8_t>(
          // c), nodes.end(), rows.end());
    // if (next_it == nodes.end())
    // {
      // assert(it != nodes.end());
      // assert(rows_it != rows.end());
      // return {it, rows_it};
    // }
    // // fmt::print("Node now: {}, row: {}, row_index: {}\n", *next_it,
        // // rows_it - rows.begin(), it - nodes.begin() + *rows_it);
    // it = next_it;
    // ++rows_it;
    // // if (use_cache)
    // // {
      // // cache_.append(c, {it, rows_it});
    // // }
  // }
  // assert(it != nodes.end());
  // assert(rows_it != rows.end());
  // return {it, rows_it};
// }

}

}

