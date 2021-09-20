#include "wordsearch_solver/dictionary_std_set/dictionary_std_set.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <range/v3/view/all.hpp>
#include <range/v3/view/transform.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <set>
#include <string>
#include <string_view>

namespace dictionary_std_set {

DictionaryStdSet::DictionaryStdSet(
    const std::initializer_list<std::string_view>& words)
    : DictionaryStdSet(ranges::views::all(words)) {}

DictionaryStdSet::DictionaryStdSet(
    const std::initializer_list<std::string>& words)
    : DictionaryStdSet(ranges::views::all(words)) {}

DictionaryStdSet::DictionaryStdSet(
    const std::initializer_list<const char*>& words)
    : DictionaryStdSet(ranges::views::all(words) |
                       ranges::views::transform([](const auto string_literal) {
                         return std::string_view{string_literal};
                       })) {}

std::size_t DictionaryStdSet::size() const { return dict_.size(); }

bool DictionaryStdSet::empty() const { return dict_.empty(); }

bool DictionaryStdSet::contains(const std::string_view word) const {
  return dict_.find(word) != dict_.end();
  // return std::binary_search(dict_.begin(), dict_.end(), key);
}

bool DictionaryStdSet::further(const std::string_view prefix) const {
  // const auto it = std::upper_bound(dict_.begin(), dict_.end(), prefix);
  const auto it = dict_.upper_bound(prefix);
  if (it == dict_.end()) {
    return false;
  }
  const auto& word = *it;
  if (word.size() < prefix.size()) {
    return false;
  }
  return std::equal(
      prefix.begin(), prefix.end(), word.begin(),
      word.begin() +
          static_cast<decltype(prefix)::difference_type>(prefix.size()));
}

std::ostream& operator<<(std::ostream& os, const DictionaryStdSet& dsv) {
  return os << fmt::format("{}", dsv.dict_);
}

} // namespace dictionary_std_set
