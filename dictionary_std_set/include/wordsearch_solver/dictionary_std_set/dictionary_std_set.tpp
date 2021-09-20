#ifndef DICTIONARY_STD_SET_TPP
#define DICTIONARY_STD_SET_TPP

#include "wordsearch_solver/dictionary_std_set/dictionary_std_set.hpp"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace dictionary_std_set {

template <class ForwardRange>
DictionaryStdSet::DictionaryStdSet(const ForwardRange& words)
    : DictionaryStdSet(words.begin(), words.end()) {}

// Actual cons that does the work
template <
    class Iterator1, class Iterator2,
    std::enable_if_t<
        std::is_same_v<typename std::iterator_traits<Iterator1>::value_type,
                       std::string_view>,
        int>>
DictionaryStdSet::DictionaryStdSet(Iterator1 first, const Iterator2 last)
    : dict_() {
  for (; first != last; ++first) {
    dict_.insert(std::string{*first});
  }
}

// Actual cons that does the work
template <
    class Iterator1, class Iterator2,
    std::enable_if_t<
        !std::is_same_v<typename std::iterator_traits<Iterator1>::value_type,
                        std::string_view>,
        int>>
DictionaryStdSet::DictionaryStdSet(Iterator1 first, const Iterator2 last)
    : dict_(first, last) {}

template <class OutputIndexIterator>
void DictionaryStdSet::contains_further(const std::string_view stem,
                                        const std::string_view suffixes,
                                        OutputIndexIterator it) const {
  // fmt::print("Dict: {}\n", dict_);
  for (const auto suffix : suffixes) {
    const std::string word = std::string{stem} + suffix;
    // fmt::print("\ncontains/further for: {}\n", word);
    const bool c = contains(word);
    const bool f = further(word);
    // fmt::print("contains/further for {}: {}/{}\n", word, c, f);
    *it++ = {c, f};
  }
}

} // namespace dictionary_std_set

#endif // DICTIONARY_STD_SET_TPP
