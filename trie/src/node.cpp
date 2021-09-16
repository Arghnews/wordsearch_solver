#include "wordsearch_solver/trie/node.hpp"

#include <algorithm>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <bitset>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <ostream>
#include <string_view>

namespace trie {

const Node* search(const Node& node, std::string_view word) {
  const Node* p = &node;

  for (; !word.empty(); word.remove_prefix(1)) {
    p = p->test(word.front());
    if (!p) {
      return nullptr;
    }
  }
  return p;
}

Node::Node(const bool is_end_of_word) : is_end_of_word_(is_end_of_word) {}

std::ostream& operator<<(std::ostream& os, const Node& node) {
  fmt::memory_buffer buff;
  fmt::format_to(buff, "{{");
  for (const auto& edge : node.edges_) {
    fmt::format_to(buff, "{}", edge.c);
  }
  fmt::format_to(buff, "{}", node.is_end_of_word() ? "|" : " ");
  fmt::format_to(buff, "}}");
  return os << fmt::to_string(buff);
}

Node* Node::add_char(const char c) {
  // assert(c >= 97 && c < 123);
  // Note comparator lambda here compares an edge and a char, not two edges
  // Insertion is sorted
  auto it = std::lower_bound(
      edges_.begin(), edges_.end(), c,
      [](const auto& edge0, const char c) { return edge0.c < c; });
  if (it == edges_.end() || it->c > c) {
    it = edges_.emplace(it, Edge{std::make_unique<Node>(), c});
  }
  assert(it->child.get() != nullptr);
  return it->child.get();
}

void Node::set_is_end_of_word(const bool is_end_of_word) {
  is_end_of_word_ = is_end_of_word;
}

/** Test if a node has an edge containing the character @p c
 *
 * @note We use linear search here. Could use binary search. For the English
 * alphabet, nodes tend to be fairly sparse and small, especially once beyond
 * the first few letters. On the massive_wordsearch benchmark,
 * using binary search is noticably (~8%) slower.
 */
const Node* Node::test(const char c) const {
  const auto it = std::find_if(edges_.begin(), edges_.end(),
                               [c](const auto& edge) { return edge.c == c; });
  // const auto it = std::lower_bound(
  // edges_.begin(), edges_.end(), c,
  // [](const auto& edge, const char c) { return edge.c < c; });
  // if (it == edges_.end() || it->c != c) {
  if (it == edges_.end()) {
    return nullptr;
  }
  return it->child.get();
}

bool Node::any() const { return !edges_.empty(); }

bool Node::is_end_of_word() const { return is_end_of_word_; }

const Node::Edges& Node::edges() const { return edges_; }

// Node::PrecedingType Node::preceding() const
// {
// return preceding_;
// }

} // namespace trie

// void print_sizes()
// {
// fmt::print("bits_: {}\n", sizeof(bits_));
// fmt::print("preceding_: {}\n", sizeof(preceding_));
// fmt::print("is_end_of_word_: {}\n", sizeof(is_end_of_word_));
// fmt::print("sizeof(Node): {}\n", sizeof(Node));
// // static_assert(sizeof(Node) <= 8);
// static_assert(alignof(Node) == 8);
// }
