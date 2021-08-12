#ifndef TRIE_HPP
#define TRIE_HPP

#include "wordsearch_solver/trie/node.hpp"
#include "wordsearch_solver/utility/flat_char_value_map.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

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

  // See trie/node.hpp before changing this, must implement proper deep copy
  Trie(const Trie &) = delete;
  Trie &operator=(const Trie &) = delete;

  Trie(const std::initializer_list<std::string_view>& words);
  Trie(const std::initializer_list<std::string>& words);
  Trie(const std::initializer_list<const char*>& words);

  template<class Iterator1, class Iterator2>
  Trie(Iterator1 first, const Iterator2 last);

  // TODO: constrain this (sfinae or concepts(>=c++20))
  // Strings should be a range of strings
  template<class Strings>
  explicit Trie(Strings&& strings_in);

  bool contains(std::string_view word) const;
  bool further(std::string_view word) const;

  template<class OutputIterator>
  void contains_further(const std::string_view stem,
      const std::string_view suffixes,
      OutputIterator contains_further_it) const;

  std::size_t size() const;
  bool empty() const;

  friend std::ostream& operator<<(std::ostream& os, const Trie& ct);

  private:

  const Node* search(std::string_view word) const;
  std::pair<Node*, bool> insert(std::string_view word);

  Node root_;
  std::size_t size_;
  mutable utility::FlatCharValueMap<const Node*> cache_;
};

namespace detail
{

bool contains(const Node& node, std::string_view word);
bool further(const Node& node, std::string_view word);

}

}

#include "wordsearch_solver/trie/trie.tpp"

#endif // TRIE_HPP
