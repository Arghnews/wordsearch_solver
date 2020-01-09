#include "dictionary_std_set.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <type_traits>
#include <vector>

DictionaryStdSet::DictionaryStdSet(const std::vector<std::string>& dict)
  : dictionary_(dict.begin(), dict.end())
{}

// no inline only here for benchmarking, remove in release?
// __attribute__((__noinline__))
bool
DictionaryStdSet::contains(const std::string& key) const
{
  return dictionary_.find(key) != dictionary_.end();
}

// __attribute__((__noinline__))
bool DictionaryStdSet::further(const std::string& prefix) const
{
  const auto it = dictionary_.upper_bound(prefix);
  if (it == dictionary_.end()) return false;

  // Compare substrings (of whichever is shortest length) for equality
  const auto& haystack_string = *it;
  const auto size = static_cast<
    std::decay_t<decltype(prefix)>::difference_type>(
      std::min(haystack_string.size(), prefix.size()));
  return std::equal(
      haystack_string.begin(), std::next(haystack_string.begin(), size),
      prefix.begin(), std::next(prefix.begin(), size));
}
