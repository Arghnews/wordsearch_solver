#ifndef UTILITY_FLAT_CHAR_VALUE_MAP_HPP
#define UTILITY_FLAT_CHAR_VALUE_MAP_HPP

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace utility {

/** A small cache for each character of a word and an associated value, such as
 * iterator into a trie for each character.
 *
 * Really this exists to overcome a potential performance limitation of the
 * interface by which the dictionary solvers plug into the solver algorithm,
 * where they would otherwise have to find their position again every iteration
 * from scratch. However, for some iterations, the stem of the word is still the
 * same, as the solver performs a breadth first search.
 *
 * A different interface for the solver dictionaries to implement may help this,
 * or simply storing more state on the solvers. What I'd @b really like is c++20
 * coroutines, allowing the solver dictionaries to write a tradional for loop
 * style solver that would keep all the appropriate state in scope managed by
 * the coroutine.
 */
template <class Value> class FlatCharValueMap {
public:
  FlatCharValueMap() = default;

  // Makes little sense to copy a cache from object to another, so just leave
  // the cache empty
  FlatCharValueMap(const FlatCharValueMap&) : FlatCharValueMap() {}
  FlatCharValueMap& operator=(const FlatCharValueMap&) { this->clear(); }

  // Move construction and move assignment both leave the cache in a default
  // constructed state, ie. as if clear() had been called. This is to prevent
  // potential bugs (if the cached values are pointers or iterators), is simple
  // to implement and unless the cache is frequently moved (it's not, or not
  // intended to be at least) will not be a performance issue.
  // FlatCharValueMap(FlatCharValueMap&&) : FlatCharValueMap() {}
  // FlatCharValueMap& operator=(FlatCharValueMap&&)
  // {
  // this->clear();
  // }

  ~FlatCharValueMap() {
    // std::cout << "FlatCharValueMap hits: " << hits << " vs misses: " <<
    // misses << " -- hits per miss: " << static_cast<double>(hits) /
    // static_cast<double>(misses) << "" << std::endl;
  }

  using NumbElementsConsumed = std::size_t;
  /** Retrieve the cached value for a word
   *
   * @param[in] word The word to lookup
   * @param[in,out] consumed Number that will be incremented by how many letters
   * are matched
   *
   * @returns Pointer to the `Value`, or `nullptr` if no letters matched
   *
   * @note gcc seems to take a perf hit when passed a string param and then a
   * string_view must be created, clang not so. Both compilers seem unable to
   * completely remove the overhead of constructing a string_view by value when
   * passed one.
   */
  inline const Value* lookup(const std::string_view& word,
                             std::size_t& consumed) {
    const auto numb_elements_consumed = this->lookup_impl(word);
    // hits += numb_elements_consumed;
    // misses += word.size() - numb_elements_consumed;
    keys_.resize(numb_elements_consumed);
    values_.resize(numb_elements_consumed);
    if (numb_elements_consumed == 0) {
      return nullptr;
    }
    consumed += numb_elements_consumed;
    return &values_[numb_elements_consumed - 1];
    // return {{numb_elements_consumed, values_[numb_elements_consumed - 1]}};
  }

  /** Add a mapping from a `char` @p key to @p value
   * @param[in] key
   * @param[in] value
   */
  inline void append(const char key, const Value& value) {
    keys_.push_back(key);
    values_.push_back(value);
  }

  /** @overload */
  inline void append(const char key, Value&& value) {
    keys_.push_back(key);
    values_.push_back(std::move(value));
  }

  /** Clears the cache */
  void clear() {
    keys_.clear();
    values_.clear();
  }

private:
  /** Lookup @p word and return how many letters are found in the cache.
   *
   * @param[in] word
   *
   * Templated (unnecessarily now) for use with string or string_view
   *
   * @returns 0 if none found
   */
  template <class Str>
  inline NumbElementsConsumed lookup_impl(const Str& word) const {
    // No elems consumed if no cached keys or searching for nothing
    if (keys_.empty() || word.empty())
      return 0;

    std::size_t i = 0;
    // Amazingly, changing from std::min to ternary in gcc takes 30% less time
    // on google bench
    const auto shorter =
        keys_.size() < word.size() ? keys_.size() : word.size();
    // const auto shorter = std::min(keys_.size(), word.size());

    // #define CHECK_AND_RETURN(index)
    // if (keys_[i + index] != word[i + index]) i += index; break;

    // for (; i + 6 <= shorter; i += 6)
    // {
    // CHECK_AND_RETURN(0);
    // CHECK_AND_RETURN(1);
    // CHECK_AND_RETURN(2);
    // CHECK_AND_RETURN(3);
    // CHECK_AND_RETURN(4);
    // CHECK_AND_RETURN(5);
    // }
    // #undef CHECK_AND_RETURN

    for (; i < shorter; ++i) {
      if (keys_[i] != word[i])
        break;
    }

    return i;
  }

  std::string keys_;
  std::vector<Value> values_;

  mutable std::size_t hits = 0;
  mutable std::size_t misses = 0;
};

} // namespace utility

#endif // UTILITY_FLAT_CHAR_VALUE_MAP_HPP
