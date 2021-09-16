#ifndef NODE_HPP
#define NODE_HPP

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <ostream>

namespace compact_trie {

/** Bitset based node representing suffixes/letters.
 */
class Node {
public:
  using PrecedingType = std::uint32_t;
  // using PrecedingType = unsigned short int;

  Node() = default;

  void add_char(char c);
  void set_preceding(std::size_t preceding);
  void set_is_end_of_word(bool is_end_of_word);

  /** @returns Number of on bits/suffixes before the bit at index @p i. Used for
   * calculating the offset into the next row for child nodes.
   * @param[in] i
   */
  std::size_t bits_on_before(std::size_t i) const;

  /** O(1) test if a suffix is present.
   * @param[in] i Suffix/letter to test, 0 == 'a', 1 == 'b'...
   */
  bool test(std::size_t i) const;

  /** @returns True if any suffixes are present/child nodes of this one exist.
   */
  bool any() const;

  bool is_end_of_word() const;

  /** @returns The number of suffixes of words in the row prior to this `Node`,
   * used for calculating the offset into the next row for child nodes.
   */
  PrecedingType preceding() const;

  friend std::ostream& operator<<(std::ostream& os, const Node& node);

private:
  std::bitset<26> bits_;
  PrecedingType preceding_;
  bool is_end_of_word_;
};

// libstdc++ with gcc 8 on linux  bitset implementation uses an unsigned long to
// back the bitset, so a bitset of size 1 is 8 bytes long. It in fact uses
// multiples of 8, ie. sizeof(std::bitset<65>) == 16
// Since Node will therefore be 8-byte aligned, may as well use std::uint32_t
// for PrecedingType

// static_assert(sizeof(std::bitset<1>) == 8);
// static_assert(sizeof(std::bitset<26>) == 8);
// static_assert(sizeof(std::bitset<65>) == 16);
// static_assert(sizeof(Node) == 16);

} // namespace compact_trie

#endif // NODE_HPP
