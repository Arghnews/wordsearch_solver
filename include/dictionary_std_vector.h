#ifndef DICTIONARY_STD_VECTOR_H
#define DICTIONARY_STD_VECTOR_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <jr_assert/jr_assert.h>

class DictionaryStdVector
{
public:

  DictionaryStdVector(const std::vector<std::string>& dict);

  template<class OutputIndexIterator>
  void contains_and_further(
      const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains,
      OutputIndexIterator further,
      OutputIndexIterator contains_and_further
      ) const;

  bool contains(const std::string& key) const;
  bool further(const std::string& key) const;
private:
  using Iterator = std::vector<std::string>::const_iterator;

  bool further_impl(
      const std::string& prefix,
      Iterator from,
      Iterator to
      ) const;
  std::pair<Iterator, Iterator> lookup_range(const std::string& key) const;

  static inline bool min_length_equal(const std::string& s1, const std::string& s2)
    noexcept;

  std::vector<std::string> dict_;
  std::map<char, std::size_t> cache_;
};

inline bool DictionaryStdVector::min_length_equal(const std::string& s1, const std::string& s2) noexcept
{
  const auto size = static_cast<std::string::difference_type>(
      std::min(s1.size(), s2.size()));
  return std::equal(
      s1.begin(), std::next(s1.begin(), size),
      s2.begin(), std::next(s2.begin(), size));
}

template<class OutputIndexIterator>
void DictionaryStdVector::contains_and_further(
    const std::string& stem_in,
    const std::string& suffixes,
    OutputIndexIterator contains_out_it,
    OutputIndexIterator further_out_it,
    OutputIndexIterator contains_and_further_out_it
    ) const
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
    // auto first = std::find(it, to, stem);

    if (first == to)
    {
      continue;
    }

    const auto contains = *first == stem;
    // const auto contains = true;

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
      *contains_and_further_out_it++ = i;
    } else if (contains)
    {
      *contains_out_it++ = i;
    } else if (further)
    {
      *further_out_it++ = i;
    }

    // it = first;
  }
  stem.pop_back();

  // return result_out;
}

#endif
