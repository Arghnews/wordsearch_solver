#ifndef DICTIONARY_STD_SET_HPP
#define DICTIONARY_STD_SET_HPP

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <ostream>
#include <string>
#include <string_view>
#include <set>

namespace dictionary_std_set
{

class DictionaryStdSet
{
public:
  DictionaryStdSet() = default;

  DictionaryStdSet(DictionaryStdSet&&) = default;
  DictionaryStdSet& operator=(DictionaryStdSet&&) = default;

  DictionaryStdSet(const DictionaryStdSet&) = delete;
  DictionaryStdSet& operator=(const DictionaryStdSet&) = delete;

  DictionaryStdSet(const std::initializer_list<std::string_view>& words);
  DictionaryStdSet(const std::initializer_list<std::string>& words);
  DictionaryStdSet(const std::initializer_list<const char*>& words);

  // Disgusting but necessary as string_view cannot be implicitly converted to
  // string
  template<class Iterator1, class Iterator2,
    std::enable_if_t<
      std::is_same_v<typename std::iterator_traits<Iterator1>::value_type, std::string_view>,
      int
    > = 0
    >
  DictionaryStdSet(Iterator1 first, const Iterator2 last);

  template<class Iterator1, class Iterator2,
    std::enable_if_t<
      !std::is_same_v<typename std::iterator_traits<Iterator1>::value_type, std::string_view>,
      int
    > = 0
    >
  DictionaryStdSet(Iterator1 first, const Iterator2 last);

  // TODO: awful SFINAE or wait until 2030 for widespread cpp20 concepts to
  // constrain this to a ForwardRange
  template<class ForwardRange>
  explicit DictionaryStdSet(ForwardRange&& words);

  std::size_t size() const;
  bool empty() const;

  template<class OutputIndexIterator>
  void contains_further(
      const std::string_view stem,
      const std::string_view suffixes,
      OutputIndexIterator contains_further) const;

  bool contains(const std::string_view key) const;
  bool further(const std::string_view key) const;

  friend std::ostream& operator<<(std::ostream&, const DictionaryStdSet&);

private:
  using Iterator = std::set<std::string>::const_iterator;

  // Needs "transparent" comparator to be able to search up string_view in a set
  // of strings, hence the less<void> specialisation
  std::set<std::string, std::less<void>> dict_;
};

}

#include "wordsearch_solver/dictionary_std_set/dictionary_std_set.tpp"

#endif // DICTIONARY_STD_SET_HPP
