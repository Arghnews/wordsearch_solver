#include "dictionary_std_set.h"

#include <algorithm>
#include <iterator>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "jr_assert/jr_assert.h"

#include "wordsearch_solver_defs.h"

DictionaryStdSet::DictionaryStdSet(const std::vector<std::string>& dict)
  : dictionary_(dict.begin(), dict.end())
{}

wordsearch_solver::Result DictionaryStdSet::contains_and_further(
    const std::string& stem_in, const std::string& suffixes) const
{
  wordsearch_solver::Result result;
  auto stem = stem_in;

  // NOTE: found in benchmarking previous version that constructing a string for
  // stem each time was a noticable expense. Consider if this again is a
  // performance problem.


  for (auto i = 0ULL; i < suffixes.size(); ++i)
  {
    stem.push_back(suffixes[i]);

    // Wonder if possible to give optimiser chance to somehow schedule these
    // together ie do both in separate arrays and then & them for the both?
    // Would that be faster than this where maybe we must wait for both?
    // Maybe change this to bitset after or something anyway rather than fat
    // heap vectors
    const auto contains = this->contains(stem);
    const auto further = this->contains_prefix(stem);
    if (contains && further)
    {
      result.contains_and_further.push_back(i);
    } else if (contains)
    {
      result.contains.push_back(i);
    } else if (further)
    {
      result.further.push_back(i);
    }

    stem.pop_back();
  }
  return result;
}

// no inline only here for benchmarking, remove in release?
__attribute__((__noinline__))
bool
DictionaryStdSet::contains(const std::string& key) const
{
  return dictionary_.find(key) != dictionary_.end();
}

__attribute__((__noinline__))
bool DictionaryStdSet::contains_prefix(const std::string& prefix) const
{
  const auto it = dictionary_.lower_bound(prefix);
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