#ifndef NODE_HPP
#define NODE_HPP

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <bitset>
#include <cstddef>
#include <ostream>

namespace compact_trie {

class Node {
public:
  using PrecedingType = unsigned short int;

  Node() = default;

  void add_char(char c);
  void set_preceding(std::size_t preceding);
  void set_is_end_of_word(bool is_end_of_word);

  std::size_t bits_on_before(std::size_t i) const;
  bool test(std::size_t i) const;
  bool any() const;
  bool is_end_of_word() const;
  PrecedingType preceding() const;
  friend std::ostream& operator<<(std::ostream& os, const Node& node);

private:
  std::bitset<26> bits_;
  PrecedingType preceding_;
  bool is_end_of_word_;
};

} // namespace compact_trie

#endif // NODE_HPP
