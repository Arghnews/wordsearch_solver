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
  // std::pair<NumbElementsConsumed, std::optional<Value>>
  // inline std::optional<std::pair<NumbElementsConsumed, Value>>
  // gcc seems to take a perf hit when passed a string param and then a
  // string_view must be created, clang not so. Both compilers seem unable to
  // completely remove the overhead of constructing a string_view by value when
  // passed one.
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

  inline void append(const char key, const Value& value) {
    keys_.push_back(key);
    values_.push_back(value);
  }

  inline void append(const char key, Value&& value) {
    keys_.push_back(key);
    values_.push_back(std::move(value));
  }

  void clear() {
    keys_.clear();
    values_.clear();
  }

private:
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

  // struct KV
  // {
  // Value v;
  // char k;
  // };
  // std::vector<KV> kvs_;

  // const TrieImpl* tp_;
  std::string keys_;
  std::vector<Value> values_;

  mutable std::size_t hits = 0;
  mutable std::size_t misses = 0;
};

} // namespace utility

#endif // UTILITY_FLAT_CHAR_VALUE_MAP_HPP
