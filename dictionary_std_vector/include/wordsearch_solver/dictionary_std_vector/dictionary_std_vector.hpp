#ifndef DICTIONARY_STD_VECTOR_HPP
#define DICTIONARY_STD_VECTOR_HPP

#include <cstddef>
#include <initializer_list>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

/** namespace dictionary_std_vector */
namespace dictionary_std_vector {

/** Solver implementation using a sorted `std::vector`.
 *
 * contains() is O(log(n))
 * further() is more complicated, and suffers from being unable to definitively
 * determine whether there may be further words. O(log(n))
 *
 * Significantly faster than the `std::set` based solver dictionary
 * implementation due to an optimisation where we find the upper limit of a
 * search, see calc_stop() and contains_further(). Still significantly slower
 * than any of the actual tries.
 */
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

  /** Actual constructor that does the work. */
  template <class Iterator1, class Iterator2>
  DictionaryStdVector(Iterator1 first, const Iterator2 last);

  // TODO: awful SFINAE or wait until 2030 for widespread cpp20 concepts to
  // constrain this to a ForwardRange
  template <class ForwardRange>
  explicit DictionaryStdVector(const ForwardRange& words);

  std::size_t size() const;
  bool empty() const;

  /** @copydoc solver::SolverDictWrapper::contains_further() */
  template <class OutputIndexIterator>
  void contains_further(const std::string_view stem,
                        const std::string_view suffixes,
                        OutputIndexIterator contains_further_it) const;

  /** @copydoc solver::SolverDictWrapper::contains() */
  bool contains(const std::string_view word) const;
  /** @copydoc solver::SolverDictWrapper::further() */
  bool further(const std::string_view word) const;

  friend std::ostream& operator<<(std::ostream&, const DictionaryStdVector&);

private:
  using Iterator = std::vector<std::string>::const_iterator;
  bool further_impl(const std::string_view key, Iterator first,
                    Iterator last) const;

  std::vector<std::string> dict_;
};

} // namespace dictionary_std_vector

#include "wordsearch_solver/dictionary_std_vector/dictionary_std_vector.tpp"

#endif // DICTIONARY_STD_VECTOR_HPP
