#ifndef TRIE_HPP
#define TRIE_HPP

#include "node.hpp"

#include <prettyprint.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// This might ACTUALLY be a case for inheritance what with the Trie being
// a Trie?
// TODO: test this with const_iterator not a std::tuple, and try a simple user
// defined struct/pointer to make trivial type to help the optimiser? Not even
// sure if "trivial" means what I think it does anyway, remove this likely..
// TODO: maybe look into units library for the ascii/index conversion stuff, as
// that has already wasted a significant amount of time with offset stuff
// TODO: std::bitset size on this system is 8 bytes, even though it need only be
// 4 bytes (26 bits) for lowercase ascii. Could see if writing own node without
// std::bitset that is 4 bytes big changes performance due to whole thing being
// approx half size (better for cache).

namespace trie
{

class Trie
{
  public:
  Trie() = default;

  Trie(Trie&&) = default;
  Trie& operator=(Trie&&) = default;

  Trie(const Trie&) = delete;
  Trie& operator=(const Trie&) = delete;

  Trie(const std::initializer_list<std::string_view>& words);
  Trie(const std::initializer_list<std::string>& words);
  Trie(const std::initializer_list<const char*>& words);

  template<class Iterator1, class Iterator2>
  Trie(Iterator1 first, const Iterator2 last);

  template<class Strings>
  explicit Trie(Strings&& strings_in);

  bool contains(std::string_view word) const;
  bool further(std::string_view word) const;

  template<class OutputIndexIterator>
  void contains_and_further(const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains_out_it,
      OutputIndexIterator further_out_it,
      OutputIndexIterator contains_and_further_out_it) const;

  std::size_t size() const;

  friend std::ostream& operator<<(std::ostream& os, const Trie& ct);

  private:
  Node root_;
  std::size_t size_;
  // mutable utility::FlatCharValueMap<const_iterator> cache_;
};

namespace detail
{

std::pair<Node*, bool> insert(Node& node, std::string_view word);
const Node* search(const Node& node, std::string_view word);
bool contains(const Node& node, std::string_view word);
bool further(const Node& node, std::string_view word);

}

}

#include "trie.tpp"

#endif // TRIE_HPP
