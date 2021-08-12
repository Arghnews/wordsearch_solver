#ifndef DICTIONARY_STD_VECTOR_HPP
#define DICTIONARY_STD_VECTOR_HPP

#include <cstddef>
#include <initializer_list>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace dictionary_std_vector {

class DictionaryStdVector {
public:
  DictionaryStdVector() = default;
  // DictionaryStdVector(const std::vector<std::string>& dict);

  DictionaryStdVector(DictionaryStdVector&&) = default;
  DictionaryStdVector& operator=(DictionaryStdVector&&) = default;

  DictionaryStdVector(const DictionaryStdVector&) = delete;
  DictionaryStdVector& operator=(const DictionaryStdVector&) = delete;

  DictionaryStdVector(const std::initializer_list<std::string_view>& words);
  DictionaryStdVector(const std::initializer_list<std::string>& words);
  DictionaryStdVector(const std::initializer_list<const char*>& words);

  template <class Iterator1, class Iterator2>
  DictionaryStdVector(Iterator1 first, const Iterator2 last);

  // TODO: awful SFINAE or wait until 2030 for widespread cpp20 concepts to
  // constrain this to a ForwardRange
  template <class ForwardRange>
  explicit DictionaryStdVector(ForwardRange&& words);

  std::size_t size() const;
  bool empty() const;

  template <class OutputIndexIterator>
  void contains_further(const std::string_view stem,
                        const std::string_view suffixes,
                        OutputIndexIterator contains_further) const;

  bool contains(const std::string_view key) const;
  bool further(const std::string_view key) const;

  friend std::ostream& operator<<(std::ostream&, const DictionaryStdVector&);

private:
  using Iterator = std::vector<std::string>::const_iterator;

  std::vector<std::string> dict_;
};

} // namespace dictionary_std_vector

#include "wordsearch_solver/dictionary_std_vector/dictionary_std_vector.tpp"

#endif // DICTIONARY_STD_VECTOR_HPP
