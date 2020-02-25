#ifndef DICTIONARY_STD_UNORDERED_MAP_H
#define DICTIONARY_STD_UNORDERED_MAP_H

#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "wordsearch_solver_defs.h"

class DictionaryStdUnorderedMap
{
public:

  template<class OutputIndexIterator>
  void contains_and_further(
      const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains,
      OutputIndexIterator further,
      OutputIndexIterator contains_and_further
      ) const;

  DictionaryStdUnorderedMap(const std::vector<std::string>& dict);

  // bool contains(const std::string& key) const;
  // bool further(const std::string& key) const;
private:
  std::unordered_map<std::string, bool> dict_;
};

template<class OutputIndexIterator>
void DictionaryStdUnorderedMap::contains_and_further(
    const std::string& stem_in,
    const std::string& suffixes,
    OutputIndexIterator contains_out_it,
    OutputIndexIterator further_out_it,
    OutputIndexIterator contains_and_further_out_it
    ) const
{

  auto stem = stem_in;

  for (auto i = 0ULL; i < suffixes.size(); ++i)
  {
    stem.push_back(suffixes[i]);

    bool further = false;
    {
      // Nasty, should have little RAII thing (how?) or just a function maybe?
      // The duplication of pop_back
      for (const auto c: std::string_view{"abcdefghijklmnopqrstuvwxyz"})
      {
        stem.push_back(c);
        const auto it = dict_.find(stem);
        if (it != dict_.end())
        {
          further = true;
          stem.pop_back();
          break;
        }
        stem.pop_back();
      }
    }

    const auto it = dict_.find(stem);
    const bool contains = it != dict_.end() && it->second;

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

    stem.pop_back();
  }
}

#endif // DICTIONARY_STD_UNORDERED_MAP_H
