#include "wordsearch_solver/dictionary_std_vector/dictionary_std_vector.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <range/v3/view/all.hpp>
#include <range/v3/view/transform.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace dictionary_std_vector {

DictionaryStdVector::DictionaryStdVector(
    const std::initializer_list<std::string_view>& words)
    : DictionaryStdVector(ranges::views::all(words)) {}

DictionaryStdVector::DictionaryStdVector(
    const std::initializer_list<std::string>& words)
    : DictionaryStdVector(ranges::views::all(words)) {}

DictionaryStdVector::DictionaryStdVector(
    const std::initializer_list<const char*>& words)
    : DictionaryStdVector(
          ranges::views::all(words) |
          ranges::views::transform([](const auto string_literal) {
            return std::string_view{string_literal};
          })) {}

std::size_t DictionaryStdVector::size() const { return dict_.size(); }

bool DictionaryStdVector::empty() const { return dict_.empty(); }

bool DictionaryStdVector::contains(const std::string_view key) const {
  return std::binary_search(dict_.begin(), dict_.end(), key);
}

bool DictionaryStdVector::further(const std::string_view prefix) const {
  const auto it = std::upper_bound(dict_.begin(), dict_.end(), prefix);
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

std::ostream& operator<<(std::ostream& os, const DictionaryStdVector& dsv) {
  return os << fmt::format("{}", dsv.dict_);
}

} // namespace dictionary_std_vector
