#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// #include <fmt/format.h>
#include "wordsearch_solver_defs.h"

#include "trie/trie.h"

struct TrieWrapper: public trie::Trie
{
  using trie::Trie::Trie;
  wordsearch_solver::Result contains_and_further(
      const std::string& stem,
      const std::string& suffixes) const
  {
    auto result = trie::Trie::contains_and_further(stem, suffixes);
    return wordsearch_solver::Result{
      std::move(result.contains),
      std::move(result.further),
      std::move(result.contains_and_further)};
  }
};

