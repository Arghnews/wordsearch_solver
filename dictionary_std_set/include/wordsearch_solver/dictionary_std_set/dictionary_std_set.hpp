#ifndef DICTIONARY_STD_SET_HPP
#define DICTIONARY_STD_SET_HPP

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <ostream>
#include <set>
#include <string>
#include <string_view>

/** namespace dictionary_std_set */
namespace dictionary_std_set {

/** Naive implementation that stores the dictionary in a `std::set`.
 *
 * contains() is log(n), but further() returns a large number of false
 * positives as we cannot do better, which is a lot of wasted work.
 */
class DictionaryStdSet {
public:
  DictionaryStdSet() = default;

  DictionaryStdSet(DictionaryStdSet&&) = default;
  DictionaryStdSet& operator=(DictionaryStdSet&&) = default;

  DictionaryStdSet(const DictionaryStdSet&) = delete;
  DictionaryStdSet& operator=(const DictionaryStdSet&) = delete;

  DictionaryStdSet(const std::initializer_list<std::string_view>& words);
  DictionaryStdSet(const std::initializer_list<std::string>& words);
  DictionaryStdSet(const std::initializer_list<const char*>& words);

  /** Constructor that does the work if given a range of `std::string_view`s.
   *
   * Disgusting but necessary as string_view cannot be implicitly converted to
   * string
   */
  template <
      class Iterator1, class Iterator2,
      std::enable_if_t<
          std::is_same_v<typename std::iterator_traits<Iterator1>::value_type,
                         std::string_view>,
          int> = 0>
  DictionaryStdSet(Iterator1 first, const Iterator2 last);

  /** Constructor that does the work */
  template <
      class Iterator1, class Iterator2,
      std::enable_if_t<
          !std::is_same_v<typename std::iterator_traits<Iterator1>::value_type,
                          std::string_view>,
          int> = 0>
  DictionaryStdSet(Iterator1 first, const Iterator2 last);

  // TODO: awful SFINAE or wait until 2030 for widespread cpp20 concepts to
  // constrain this to a ForwardRange
  template <class ForwardRange>
  explicit DictionaryStdSet(const ForwardRange& words);

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

  friend std::ostream& operator<<(std::ostream&, const DictionaryStdSet&);

private:
  using Iterator = std::set<std::string>::const_iterator;

  /** @note Needs "transparent" comparator to be able to search up
   * `std::string_view` in a set of strings, hence the `std::less<void>`
   * specialisation
   */
  std::set<std::string, std::less<void>> dict_;
};

} // namespace dictionary_std_set

#include "wordsearch_solver/dictionary_std_set/dictionary_std_set.tpp"

#endif // DICTIONARY_STD_SET_HPP
