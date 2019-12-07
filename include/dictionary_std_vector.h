#ifndef DICTIONARY_STD_VECTOR_H
#define DICTIONARY_STD_VECTOR_H

#include <cstddef>
#include <string>
#include <vector>
#include <map>

#include "wordsearch_solver_defs.h"

class DictionaryStdVector
{
public:

  void contains_and_further(
      const std::string& stem,
      const std::string& suffixes,
      wordsearch_solver::Result& result_out) const;

  DictionaryStdVector(const std::vector<std::string>& dict);

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

  std::vector<std::string> dict_;
  std::map<char, std::size_t> cache_;
};

#endif // DICTIONARY_STD_VECTOR_H
