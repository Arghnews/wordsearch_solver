#include "dictionary_std_unordered_map.h"

#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>

DictionaryStdUnorderedMap::DictionaryStdUnorderedMap(
    const std::vector<std::string>& dict)
{
  decltype(dict_) m;
  for (const auto& word: dict)
  {
    std::string w;
    w.reserve(word.size());

    for (auto it = word.begin(); it != word.end(); ++it)
    {
      w.push_back(*it);
      const bool end_of_word = std::next(it) == word.end();

      auto [i, _] = m.insert({w, end_of_word});
      // If the entry was marked as the end of a word or the inserted word is,
      // set the bool to true - make it an end of a word
      i->second |= end_of_word;
    }
    w.clear();
  }
  dict_ = std::move(m);
}

// no inline only here for benchmarking, remove in release?
// __attribute__((__noinline__))
// bool
// DictionaryStdUnorderedMap::contains(const std::string& key) const
// {
  // return dictionary_.find(key) != dictionary_.end();
// }

// // __attribute__((__noinline__))
// bool DictionaryStdUnorderedMap::further(const std::string& prefix) const
// {
  // const auto it = dictionary_.upper_bound(prefix);
  // if (it == dictionary_.end()) return false;

  // // Compare substrings (of whichever is shortest length) for equality
  // const auto& haystack_string = *it;
  // const auto size = static_cast<
    // std::decay_t<decltype(prefix)>::difference_type>(
      // std::min(haystack_string.size(), prefix.size()));
  // return std::equal(
      // haystack_string.begin(), std::next(haystack_string.begin(), size),
      // prefix.begin(), std::next(prefix.begin(), size));
// }
