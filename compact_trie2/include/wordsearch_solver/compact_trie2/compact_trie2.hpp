#ifndef COMPACT_TRIE2_COMPACT_TRIE2_HPP
#define COMPACT_TRIE2_COMPACT_TRIE2_HPP

#include "wordsearch_solver/compact_trie2/compact_trie2_iterator_typedefs.hpp"
#include "wordsearch_solver/compact_trie2/empty_node_view.hpp"
#include "wordsearch_solver/compact_trie2/full_node_view.hpp"
#include "wordsearch_solver/utility/flat_char_value_map.hpp"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace compact_trie2
{

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

  template<class OutputIterator>
  void contains_further(const std::string_view stem,
      const std::string_view suffixes,
      OutputIterator contains_further_it) const;

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
  mutable utility::FlatCharValueMap<std::pair<DataIterator, RowIterator>> cache_;
};

}

#include "wordsearch_solver/compact_trie2/compact_trie2.tpp"

#endif // COMPACT_TRIE2_COMPACT_TRIE2_HPP
