#ifndef COMPACT_TRIE_HPP
#define COMPACT_TRIE_HPP

#include "utility/utility.hpp"
#include "utility/flat_char_value_map.hpp"
#include "node.hpp"

#include <prettyprint.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <range/v3/view/subrange.hpp>

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

// This might ACTUALLY be a case for inheritance what with the CompactTrie being
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

namespace compact_trie
{

class CompactTrie
{
  public:

  using Nodes = std::vector<Node>;
  using NodesIterator = std::vector<Node>::const_iterator;
  using Rows = std::vector<NodesIterator>;
  using RowsIterator = Rows::const_iterator;
  using const_iterator = std::tuple<NodesIterator, RowsIterator>;
  // static_assert(std::is_trivially_copyable_v<const_iterator>);

  CompactTrie() = default;

  CompactTrie(CompactTrie&&) = default;
  CompactTrie& operator=(CompactTrie&&) = default;

  CompactTrie(const CompactTrie&) = delete;
  CompactTrie& operator=(const CompactTrie&) = delete;

  CompactTrie(const std::initializer_list<std::string_view>& words);
  CompactTrie(const std::initializer_list<std::string>& words);
  CompactTrie(const std::initializer_list<const char*>& words);

  template<class Iterator1, class Iterator2>
  CompactTrie(Iterator1 first, const Iterator2 last);

  template<class Strings>
  explicit CompactTrie(Strings&& strings_in);

  bool contains(std::string_view word,
      ranges::subrange<NodesIterator> nodes,
      ranges::subrange<RowsIterator> rows) const;
  bool contains(std::string_view word) const;

  bool further(std::string_view word,
      ranges::subrange<NodesIterator> nodes,
      ranges::subrange<RowsIterator> rows) const;
  bool further(std::string_view word) const;

  template<class OutputIndexIterator>
  void contains_and_further(const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains_out_it,
      OutputIndexIterator further_out_it,
      OutputIndexIterator contains_and_further_out_it) const;

  std::size_t size() const;

  private:
  CompactTrie::const_iterator search(std::string_view word,
      ranges::subrange<CompactTrie::NodesIterator> nodes,
      ranges::subrange<CompactTrie::RowsIterator> rows) const;


  friend std::ostream& operator<<(std::ostream& os, const CompactTrie& ct);

  Nodes nodes_;
  Rows rows_;
  std::size_t size_;
  mutable utility::FlatCharValueMap<const_iterator> cache_;
};

}

#include "compact_trie.tpp"

#endif // COMPACT_TRIE_HPP
