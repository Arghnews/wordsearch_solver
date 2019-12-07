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

namespace
{
bool min_length_equal(const std::string& s1, const std::string& s2) noexcept
{
  const auto size = static_cast<std::string::difference_type>(
      std::min(s1.size(), s2.size()));
  return std::equal(
      s1.begin(), std::next(s1.begin(), size),
      s2.begin(), std::next(s2.begin(), size));
}
}

void DictionaryStdVector::contains_and_further(
    const std::string& stem_in,
    const std::string& suffixes,
    wordsearch_solver::Result& result_out) const
{
  // wordsearch_solver::Result result;
  // auto suffixes = suffixes_in;
  // std::sort(suffixes.begin(), suffixes.end());
  // JR_ASSERT(std::is_sorted(suffixes.begin(), suffixes.end()), "{}", suffixes);
  auto stem = stem_in;

  // NOTE: found in benchmarking previous version that constructing a string for
  // stem each time was a noticable expense. Consider if this again is a
  // performance problem.

  // auto it = std::lower_bound(dict_.begin(), dict_.end(), stem_in);
  // TODO ->>>>>>>>>>>>>>>>>> : need to think about this one but too tired atm,
  // think we need
  // something similar to
  // further_impl!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // if (it == dict_.end()) return;

  const auto [from, to] = lookup_range(stem);
  const auto it = std::upper_bound(from, to, stem);
  if (it == to) return;

  stem.push_back('\0');
  for (auto i = 0ULL; i < suffixes.size(); ++i)
  {
    stem.back() = suffixes[i];

    // TODO: replace using std::partition_point to look from there every time?
    auto first = std::lower_bound(it, to, stem);

    if (first == to)
    {
      continue;
    }

    const auto contains = *first == stem;
    // const auto contains2 = this->contains(stem);
    // JR_ASSERT(contains == contains2);
    // JR_ASSERT(contains == this->contains(stem),
        // "Should be: {} but isn't. Word is {}", contains, stem);

    if (contains)
    {
      ++first;
    }

    // Vector end of first letter places
    // Potentially cache idea per letter too, could be as good as on trie here

    // Currently as it stands - for certain sets of test data - the 2nd version
    // is faster. The lower_bound searches take too long.
    // const auto further = this->further_impl(stem, first, to);
    const bool further = first != to ? min_length_equal(*first, stem) : false;

    if (contains && further)
    {
      result_out.contains_and_further.push_back(i);
    } else if (contains)
    {
      result_out.contains.push_back(i);
    } else if (further)
    {
      result_out.further.push_back(i);
    }

    // it = first;
  }
  stem.pop_back();

  // return result_out;
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
