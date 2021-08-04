#ifndef DICTIONARY_STD_VECTOR_TPP
#define DICTIONARY_STD_VECTOR_TPP

#include "wordsearch_solver/dictionary_std_vector/dictionary_std_vector.hpp"

#include <algorithm>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

namespace dictionary_std_vector
{

// Actual cons that does the work
template<class Iterator1, class Iterator2>
DictionaryStdVector::DictionaryStdVector(Iterator1 first, const Iterator2 last)
: dict_(first, last)
{
  std::sort(dict_.begin(), dict_.end());
  dict_.erase(std::unique(dict_.begin(), dict_.end()), dict_.end());
}

template<class ForwardRange>
DictionaryStdVector::DictionaryStdVector(ForwardRange&& words)
: DictionaryStdVector(words.begin(), words.end())
{}

template<class OutputIndexIterator>
void DictionaryStdVector::contains_further(const std::string_view stem,
    const std::string_view suffixes, OutputIndexIterator it) const
{
  // fmt::print("Dict: {}\n", dict_);
  for (const auto suffix: suffixes)
  {
    const std::string word = std::string{stem} + suffix;
    // fmt::print("\ncontains/further for: {}\n", word);
    const bool c = contains(word);
    const bool f = further(word);
    // fmt::print("contains/further for {}: {}/{}\n", word, c, f);
    *it++ = {c, f};
  }
}

}

#endif // DICTIONARY_STD_VECTOR_TPP
