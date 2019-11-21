#ifndef DICTIONARY_STD_SET_H
#define DICTIONARY_STD_SET_H

#include <filesystem>
#include <set>
#include <string>
#include <vector>

class DictionaryStdSet
{
public:

  bool contains(const std::string& key) const;
  bool contains_prefix(const std::string& prefix) const;

  DictionaryStdSet(const std::vector<std::string>& dict);
private:
  std::set<std::string> dictionary_;
};

#endif // DICTIONARY_STD_SET_H
