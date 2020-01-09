#ifndef DICTIONARY_STD_SET_H
#define DICTIONARY_STD_SET_H

#include <set>
#include <string>
#include <vector>

#include "wordsearch_solver_defs.h"

class DictionaryStdSet
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

  DictionaryStdSet(const std::vector<std::string>& dict);

  bool contains(const std::string& key) const;
  bool further(const std::string& key) const;
private:
  std::set<std::string> dictionary_;
};

template<class OutputIndexIterator>
void DictionaryStdSet::contains_and_further(
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
    const auto contains = this->contains(stem);
    const auto further = this->further(stem);
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

#endif // DICTIONARY_STD_SET_H
