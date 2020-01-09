#include "dictionary_std_vector.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <type_traits>
#include <map>
#include <utility>
#include <vector>

#include <jr_assert/jr_assert.h>

#include "wordsearch_solver_defs.h"

DictionaryStdVector::DictionaryStdVector(const std::vector<std::string>& dict)
  : dict_(dict)
  , cache_()
{
  // JR_ASSERT(std::is_sorted(dict_.begin(), dict_.end()));
  // JR_ASSERT(dict.size() > 1);

  // if (dict.size() >

  // for (auto i = 0ULL; i < dict.size(); ++i)
  // std::vector<std::string>

  for (auto i = dict.size(); i-- > 0;)
  {
    const auto& word = dict[i];
    JR_ASSERT(!word.empty());
    cache_[word[0]] = i;
  }

  // for (const auto [word, i]: cache_)
  // {
    // std::cout << word << "->" << i << "\n";
  // }
}

std::pair<DictionaryStdVector::Iterator, DictionaryStdVector::Iterator>
DictionaryStdVector::lookup_range(const std::string& key) const
{
  if (key.empty())
  {
    return {dict_.begin(), dict_.end()};
  }
  const auto first_it = cache_.find(key[0]);
  if (first_it == cache_.end())
  {
    return {dict_.end(), dict_.end()};
  }
  const auto first = first_it->second;
  const auto last = std::next(first_it) == cache_.end() ? dict_.size() : std::next(first_it)->second;

  // std::cout << "[" << first << ", " << last << "]" << "\n";
  // std::cout << "[" << dict_.at(first) << ", " << (
      // last == dict_.size() ? "dict.size(), empty" : dict_.at(last)
      // ) << "]" << "\n";

  using T = Iterator::difference_type;
  return {
    std::next(dict_.begin(), static_cast<T>(first)),
    std::next(dict_.begin(), static_cast<T>(last))
  };
}

// no inline only here for benchmarking, remove in release?
// __attribute__((__noinline__))
bool
DictionaryStdVector::contains(const std::string& key) const
{
  JR_ASSERT(!key.empty(), "Lookup for empty key - probably a bug?");
  const auto [first, last] = lookup_range(key);
  return std::binary_search(first, last, key);
}

bool DictionaryStdVector::further_impl(
    const std::string& prefix,
    DictionaryStdVector::Iterator from,
    DictionaryStdVector::Iterator to
    ) const
{
  const auto it = std::lower_bound(from, to, prefix);
  if (it == to)
  {
    return false;
  }
  return min_length_equal(*it, prefix);
}

// __attribute__((__noinline__))
bool DictionaryStdVector::further(const std::string& prefix) const
{
  JR_ASSERT(!prefix.empty(), "Lookup for empty key - probably a bug?");
  const auto [from, to] = lookup_range(prefix);
  if (from == to)
  {
    return false;
  }
  return further_impl(prefix, from, to);
}
