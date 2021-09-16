#include "wordsearch_solver/compact_trie/node.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <bitset>
#include <cassert>
#include <cstddef>
#include <limits>
#include <ostream>

// NOTE: I hate all the hard coded 97s and 123s for ascii lowercase start/end.
// Would be nice to have some kind of abstraction over that.

namespace compact_trie {

std::ostream& operator<<(std::ostream& os, const Node& node) {
  const auto letter_bits = node.bits_;
  const auto size = letter_bits.size();
  fmt::memory_buffer buff;
  fmt::format_to(buff, "{{");
  for (auto i = 0ULL; i < size; ++i) {
    if (letter_bits.test(static_cast<std::size_t>(i))) {
      fmt::format_to(buff, "{}", static_cast<char>(i + 97));
    }
  }
  fmt::format_to(buff, "{}", node.is_end_of_word() ? "|" : " ");
  fmt::format_to(buff, "{}", node.preceding());
  fmt::format_to(buff, "}}");
  return os << fmt::to_string(buff);
}

void Node::add_char(const char c) {
  assert(c >= 97 && c < 123);
  bits_.set(static_cast<std::size_t>(c - 97));
}

void Node::set_preceding(const std::size_t preceding) {
  assert(preceding < std::numeric_limits<PrecedingType>::max());
  preceding_ = static_cast<PrecedingType>(preceding);
}

void Node::set_is_end_of_word(const bool is_end_of_word) {
  is_end_of_word_ = is_end_of_word;
}

std::size_t Node::bits_on_before(std::size_t i) const {
  assert(i >= 97 && i < 123);
  // fmt::print("This node: {}\n", *this);
  // fmt::print("bits_on_before called with i: {}\n", i);
  // fmt::print("bits_: {}\n", bits_);
  // fmt::print("(bits_.size()): {}\n", bits_.size());
  // fmt::print("(bits_.size() - i): {}\n", bits_.size() - i);
  // fmt::print("(bits_.size() - i - 97): {}\n", bits_.size() - i - 97);
  // fmt::print("(bits_.size() - i - 97 + 1): {}\n", bits_.size() - i - 97 + 1);
  // fmt::print("(bits_.size() - (i - 97)): {}\n", bits_.size() - (i - 97));
  return (bits_ << (bits_.size() - (i - 97))).count();
}

bool Node::test(std::size_t i) const {
  assert(i >= 97 && i < 123);
  return bits_.test(i - 97);
}

bool Node::any() const { return bits_.any(); }

bool Node::is_end_of_word() const { return is_end_of_word_; }

Node::PrecedingType Node::preceding() const { return preceding_; }

} // namespace compact_trie

// void print_sizes()
// {
// fmt::print("bits_: {}\n", sizeof(bits_));
// fmt::print("preceding_: {}\n", sizeof(preceding_));
// fmt::print("is_end_of_word_: {}\n", sizeof(is_end_of_word_));
// fmt::print("sizeof(Node): {}\n", sizeof(Node));
// // static_assert(sizeof(Node) <= 8);
// static_assert(alignof(Node) == 8);
// }
