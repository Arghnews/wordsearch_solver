#ifndef FLAT_CHAR_VALUE_MAP_H
#define FLAT_CHAR_VALUE_MAP_H

#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

template<class Value>
class FlatCharValueMap
{
  public:
  FlatCharValueMap() = default;

  FlatCharValueMap(const FlatCharValueMap&) = delete;
  FlatCharValueMap& operator=(const FlatCharValueMap&) = delete;

  // Move construction and move assignment both leave the cache in a default
  // constructed state, ie. as if clear() had been called. This is to prevent
  // potential bugs (if the cached values are pointers or iterators), is simple
  // to implement and unless the cache is frequently moved (it's not, or not
  // intended to be at least) will not be a performance issue.
  FlatCharValueMap(FlatCharValueMap&&) : FlatCharValueMap() {}
  FlatCharValueMap& operator=(FlatCharValueMap&&)
  {
    this->clear();
  }

  ~FlatCharValueMap()
  {
    // std::cout << "FlatCharValueMap hits: " << hits << " vs misses: " << misses <<
      // " -- hits per miss: " <<
      // static_cast<double>(hits) / static_cast<double>(misses) << "" << "\n";
  }

  using NumbElementsConsumed = std::size_t;
  // std::pair<NumbElementsConsumed, std::optional<Value>>
  std::optional<std::pair<NumbElementsConsumed, Value>>
  lookup(const std::string_view word)
  {
    const auto numb_elements_consumed = this->lookup_impl(word);
    hits += numb_elements_consumed;
    misses += word.size() - numb_elements_consumed;
    keys_.resize(numb_elements_consumed);
    values_.resize(numb_elements_consumed);
    if (numb_elements_consumed == 0)
    {
      return {};
    }
    return {{numb_elements_consumed, values_[numb_elements_consumed - 1]}};
  }

  void append(const char key, Value&& value)
  {
    keys_.push_back(key);
    values_.push_back(std::forward<Value>(value));
  }

  void clear()
  {
    keys_.clear();
    values_.clear();
  }

  private:

  NumbElementsConsumed lookup_impl(const std::string_view word) const
  {
    // No elems consumed if no cached keys or searching for nothing
    if (keys_.empty() || word.empty()) return 0;

    std::size_t i = 0;
    const auto shorter = std::min(keys_.size(), word.size());
    for (; i < shorter; ++i)
    {
      if (keys_[i] != word[i]) break;
    }

    return i;
  }

  // const TrieImpl* tp_;
  std::string keys_;
  std::vector<Value> values_;

  mutable std::size_t hits = 0;
  mutable std::size_t misses = 0;
};

#endif // FLAT_CHAR_VALUE_MAP_H
