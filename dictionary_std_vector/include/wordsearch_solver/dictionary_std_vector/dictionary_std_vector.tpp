#ifndef DICTIONARY_STD_VECTOR_TPP
#define DICTIONARY_STD_VECTOR_TPP

#include "wordsearch_solver/dictionary_std_vector/dictionary_std_vector.hpp"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace dictionary_std_vector {

// Actual cons that does the work
template <class Iterator1, class Iterator2>
DictionaryStdVector::DictionaryStdVector(Iterator1 first, const Iterator2 last)
    : dict_(first, last) {
  std::sort(dict_.begin(), dict_.end());
  dict_.erase(std::unique(dict_.begin(), dict_.end()), dict_.end());
}

template <class ForwardRange>
DictionaryStdVector::DictionaryStdVector(ForwardRange&& words)
    : DictionaryStdVector(words.begin(), words.end()) {}

// To allow binary searching more efficiently when using a word and suffixes, we
// need to be able to calculate lower and upper limits on the search in a
// lexicographically sorted list of words. std::lower_bound(word) gives the
// start. This will calculate the end. Returns empty optional if no value ie.
// the end iterator should be used.
// Usually this is simple ie. zap -> zaq (increment last)
// But if last is the highest allowed value, then we must keep removing the last
// element until we have an end that isn't the highest allowed.
inline std::optional<std::string> calc_stop(std::string word) {
  const auto highest_domain_value = std::numeric_limits<char>::max();
  for (auto it = word.rbegin(); it != word.rend(); ++it) {
    if (*it != highest_domain_value) {
      ++(*it);
      return word;
    }
    word.pop_back();
  }
  return {};
}

template <class OutputIndexIterator>
void DictionaryStdVector::contains_further(const std::string_view stem,
                                           const std::string_view suffixes,
                                           OutputIndexIterator it) const {
  // NOTE: if you sort suffixes, must remember original order to write to output
  // iterator in!

  const auto first = std::lower_bound(dict_.begin(), dict_.end(), stem);
  if (first == dict_.end()) {
    for ([[maybe_unused]] const auto& _ : suffixes) {
      *it++ = {false, false};
    }
    return;
  }

  const auto last = [&]() {
    const auto last_item = calc_stop(std::string{stem});
    if (!last_item) {
      return dict_.end();
    }
    return std::upper_bound(first, dict_.end(), *last_item);
  }();

  for (const auto suffix : suffixes) {
    const std::string word = std::string{stem} + suffix;
    // fmt::print("\ncontains/further for: {}\n", word);
    const bool c = std::binary_search(first, last, word);
    const bool f = this->further_impl(word, first, last);
    // fmt::print("contains/further for {}: {}/{}\n", word, c, f);
    *it++ = {c, f};
  }
}

} // namespace dictionary_std_vector

#endif // DICTIONARY_STD_VECTOR_TPP
