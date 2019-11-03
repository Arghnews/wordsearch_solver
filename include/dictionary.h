#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <filesystem>
#include <string>
#include <optional>

#include "wordsearch_solver_defs.h"

namespace wordsearch_solver
{

class Dictionary
{
private:
  using Dict = std::set<std::string>;
public:
  using Iterator = Dict::const_iterator;
  std::optional<const Iterator> find(const std::string& key) const;
  bool contains_prefix(
      const std::string& prefix,
      std::optional<const Iterator> start_hint) const;
  // For now just this, hacky
  Iterator end() const;
  Dictionary(const std::filesystem::path& dictionary_path);
private:
  Dict dictionary_;
};

} // wordsearch_solver

#endif // DICTIONARY_H
