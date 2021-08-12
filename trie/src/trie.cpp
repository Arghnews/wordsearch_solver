#include "wordsearch_solver/trie/trie.hpp"
#include "wordsearch_solver/trie/node.hpp"

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
#include <vector>

// This might ACTUALLY be a case for inheritance what with the Trie being
// a Trie?
// TODO: test this with const_iterator not a std::tuple, and try a simple user
// defined struct/pointer to make trivial type to help the optimiser? Not even
// sure if "trivial" means what I think it does anyway, remove this likely..
// TODO: maybe look into units library for the ascii/index conversion stuff, as
// that has already wasted a significant amount of time with offset stuff

namespace trie {

Trie::Trie(const std::initializer_list<std::string_view>& words)
    : Trie(ranges::views::all(words)) {}

Trie::Trie(const std::initializer_list<std::string>& words)
    : Trie(ranges::views::all(words)) {}

Trie::Trie(const std::initializer_list<const char*>& words)
    : Trie(ranges::views::all(words) |
           ranges::views::transform([](const auto string_literal) {
             return std::string_view{string_literal};
           })) {}

bool Trie::contains(const std::string_view word) const {
  return detail::contains(root_, word);
}

bool Trie::further(const std::string_view word) const {
  return detail::further(root_, word);
}

std::size_t Trie::size() const { return size_; }

bool Trie::empty() const { return size_ == 0; }

std::ostream& operator<<(std::ostream& os, const Trie& ct) {
  fmt::memory_buffer buff{};
  fmt::format_to(buff, "Size: {}\n", ct.size());
  std::vector<const Node*> nodes{&ct.root_};
  std::vector<const Node*> next_row;
  while (!nodes.empty()) {
    for (const auto* node : nodes) {
      fmt::format_to(buff, "{}", *node);
      for (const auto& edge : node->edges()) {
        next_row.push_back(edge.child.get());
      }
    }
    fmt::format_to(buff, "\n");
    nodes = std::move(next_row);
  }
  return os << fmt::to_string(buff);
}

const Node* Trie::search(std::string_view word) const {
  const Node* p = &root_;

  const bool use_cache = true;
  std::size_t i = 0;
  if (use_cache) {
    const auto* cached_result = cache_.lookup(word, i);
    if (cached_result) {
      word.remove_prefix(i);
      p = *cached_result;
    }
  }

  for (; !word.empty(); word.remove_prefix(1)) {
    // fmt::print("p: {}\n", *p);
    const Node* next = p->test(word.front());
    if (!next) {
      // fmt::print("next is nullptr, ret\n");
      return nullptr;
    }
    // fmt::print("next: {}\n", *next);
    p = next;
    if (use_cache)
      cache_.append(word.front(), p);
  }
  return p;
}

std::pair<Node*, bool> Trie::insert(std::string_view word) {
  Node* p = &root_;

  for (; !word.empty(); word.remove_prefix(1)) {
    p = p->add_char(word.front());
  }
  if (!p->is_end_of_word()) {
    p->set_is_end_of_word(true);
    return {p, true};
  }
  return {p, false};
}

namespace detail {

bool contains(const Node& node, const std::string_view word) {
  const auto* p = search(node, word);
  return p && p->is_end_of_word();
}

bool further(const Node& node, const std::string_view word) {
  const auto* p = search(node, word);
  return p && p->any();
}
} // namespace detail

} // namespace trie
