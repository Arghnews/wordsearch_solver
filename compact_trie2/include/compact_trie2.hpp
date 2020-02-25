#ifndef COMPACT_TRIE2_HPP
#define COMPACT_TRIE2_HPP

#include "compact_trie2_iterator_typedefs.hpp"
#include "empty_node_view.hpp"
#include "flat_char_value_map.hpp"
#include "full_node_view.hpp"

#include <range/v3/action/push_back.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/is_sorted.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/functional/identity.hpp>
#include <range/v3/iterator/access.hpp>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/iterator/operations.hpp>
#include <range/v3/iterator/traits.hpp>
#include <range/v3/range/access.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range/primitives.hpp>
#include <range/v3/view/adaptor.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/facade.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/slice.hpp>
// #include <range/v3/view/subrange.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>
#include <range/v3/view/zip.hpp>

#include <prettyprint.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <limits>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <variant>

class CompactTrie2
{
  public:

  CompactTrie2() = default;

  CompactTrie2(CompactTrie2&&) = default;
  CompactTrie2& operator=(CompactTrie2&&) = default;

  CompactTrie2(const CompactTrie2&) = delete;
  CompactTrie2& operator=(const CompactTrie2&) = delete;

  CompactTrie2(const std::initializer_list<std::string_view>& words);
  CompactTrie2(const std::initializer_list<std::string>& words);
  CompactTrie2(const std::initializer_list<const char*>& words);

  template<class Iterator1, class Iterator2>
  CompactTrie2(Iterator1 first, const Iterator2 last);

  // TODO: awful SFINAE or wait until 2030 for widespread cpp20 concepts to
  // constrain this to a ForwardRange
  template<class ForwardRange>
  explicit CompactTrie2(ForwardRange&& words);

  std::size_t size() const;

  std::size_t data_size() const;

  bool empty() const;

  bool contains(const std::string_view word) const;

  bool further(const std::string_view word) const;

  template<class OutputIndexIterator>
  void contains_and_further(const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains_out_it,
      OutputIndexIterator further_out_it,
      OutputIndexIterator contains_and_further_out_it) const;

  // it -> data iterator
  // rows_it -> the row of that iterator (will be ++ to get the next row start)
  // i -> index, if i == word.size() found end

  friend std::ostream& operator<<(std::ostream& os, const CompactTrie2& ct);

  private:

  template<class T>
  void init(T&& words_view);

  void non_templated_rest_of_init();

  std::tuple<std::size_t, DataIterator, RowIterator>
  search(const std::string_view word, DataIterator it, RowIterator rows_it) const;

  std::vector<std::uint8_t> data_;
  std::vector<std::size_t> rows_;
  std::size_t size_;
  mutable FlatCharValueMap<std::pair<DataIterator, RowIterator>> cache_;
};

#include "compact_trie2.tpp"

#endif // COMPACT_TRIE2_H
