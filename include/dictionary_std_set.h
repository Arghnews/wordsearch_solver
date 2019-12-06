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

  void contains_and_further(
      const std::string& stem,
      const std::string& suffixes,
      wordsearch_solver::Result& result_out) const;

  DictionaryStdSet(const std::vector<std::string>& dict);

  bool contains(const std::string& key) const;
  bool further(const std::string& key) const;
private:
  std::set<std::string> dictionary_;
};

#endif // DICTIONARY_STD_SET_H
