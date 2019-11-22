#ifndef DICTIONARY_STD_SET_H
#define DICTIONARY_STD_SET_H

#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include "wordsearch_solver_defs.h"

class DictionaryStdSet
{
public:

  wordsearch_solver::Result contains_and_further(std::string tail_word,
      const std::string& suffixes) const;

  DictionaryStdSet(const std::vector<std::string>& dict);
private:
  bool contains(const std::string& key) const;
  bool contains_prefix(const std::string& prefix) const;
  std::set<std::string> dictionary_;
};

#endif // DICTIONARY_STD_SET_H
