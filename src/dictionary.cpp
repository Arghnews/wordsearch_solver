#include "dictionary.h"

#include <algorithm>
#include <type_traits>
#include <iterator>
#include <fstream>
#include <filesystem>
#include <string>
#include <optional>

#include "jr_assert.h"

using namespace wordsearch_solver;

namespace
{

std::set<std::string> readlines(const std::filesystem::path& p)
{
  std::set<std::string> lines;
  std::ifstream f{p};
  // TODO: fix jr_assert.h includes to print filesystem path?
  JR_ASSERT(f, "Error reading file {}", p.string());
  for (std::string line; std::getline(f, line);)
  {
//    lines.emplace_back(std::move(line));
    lines.emplace(std::move(line));
  }
  return lines;
}

}

Dictionary::Dictionary(const std::filesystem::path& dictionary_path)
  : dictionary_(::readlines(dictionary_path))
{}

__attribute__((__noinline__))
std::optional<const Dictionary::Iterator>
Dictionary::find(const std::string& key) const
{
  const auto it = dictionary_.find(key);
  if (it == dictionary_.end())
    return {};
  return it;
}

__attribute__((__noinline__))
bool Dictionary::contains_prefix(
    const std::string& prefix,
    [[maybe_unused]] std::optional<const Iterator> start_hint) const
{
  const auto it = dictionary_.lower_bound(prefix);
  if (it == dictionary_.end()) return false;
  const auto& haystack_string = *it;
  const auto size = static_cast<
    std::decay_t<decltype(prefix)>::difference_type>(
      std::min(haystack_string.size(), prefix.size()));
  return std::equal(
      haystack_string.begin(), std::next(haystack_string.begin(), size),
      prefix.begin(), std::next(prefix.begin(), size));
}

Dictionary::Iterator Dictionary::end() const
{
  return dictionary_.end();
}
