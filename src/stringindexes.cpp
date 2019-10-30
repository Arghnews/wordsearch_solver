#include "stringindexes.h"
#include "wordsearch_solver.h"
#include "jr_assert.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

namespace wordsearch_solver
{
//  using Grid = std::shared_ptr<std::vector<std::string>>;
//  using Dictionary = std::vector<std::string>;
//  using Index = std::pair<std::size_t, std::size_t>;
//  using Indexes = std::vector<Index>;

  StringIndex::StringIndex(const Grid& grid, std::string string,
              Indexes indexes)
    : string_(std::move(string)), indexes_(std::move(indexes))
  {
    // FIXME: these!
    JR_ASSERT(string_.size() == indexes_.size());
    JR_ASSERT(string_ == indexes_to_word(grid, indexes_));
  }

  std::size_t StringIndex::size() const
  {
    return string_.size();
  }

  const std::string& StringIndex::string() const
  {
    return string_;
  }

  // This feels and is used in a leaky implementation detail kinda way.
  // But the "proper" way I suspect is making iterators and bla bla and seems
  // a lot of boilerplate to accomplish the same thing and is unneeded here.
  const Indexes& StringIndex::indexes() const
  {
    return indexes_;
  }

// Need to think of simplest way with checks to turn a stringindex into a printed grid
// And then find out how to do the InequalityMixin thing
// And think of nice way to expose this as an interface
// Don't overcomplicate it!
// TODO: move delim default value to header file

std::string stringindex_to_grid_string(
    const Grid& gridp,
    const StringIndex& si,
    const std::string_view spacer_delim = " ")
{
  const auto& grid = *gridp;
  const auto newline = "\n";
  std::string s;

  using std::begin;
  using std::end;
  /* More generic, less readable?.. */
  for (auto i_it = begin(grid); i_it != end(grid); std::advance(i_it, 1))
  {
    bool prepend_delim = false;
    for (auto j_it = begin(*i_it); j_it != end(*i_it); std::advance(j_it, 1))
    {
      if (prepend_delim)
        s.append(spacer_delim);
      prepend_delim = true;
      const auto i = static_cast<std::size_t>(std::distance(begin(grid), i_it));
      const auto j = static_cast<std::size_t>(std::distance(begin(*i_it), j_it));

      // If the index is in the word found, draw the letter in the grid by
      // it back onto s. Otherwise push an empty space.
      if (std::find(si.indexes().begin(), si.indexes().end(), Index{i, j})
          != si.indexes().end())
      {
        s.push_back(grid.at(i).at(j));
      } else
      {
        s.push_back(' ');
      }
    }
    s.append(newline);
  }
  return s;
}

// Sort by length (descending, longest first) then lexicographic)
bool operator<(const StringIndex& si1, const StringIndex& si2)
{
  if (si1.string_.size () != si2.string_.size())
    return si1.string_.size() > si2.string_.size();
  return si1.string_ < si2.string_;
}

bool operator==(const StringIndex& si1, const StringIndex& si2)
{
  return std::tie(si1.string_, si1.indexes_) ==
      std::tie(si2.string_, si2.indexes_);
}

bool operator!=(const StringIndex& si1, const StringIndex& si2)
{
  return !(si1 == si2);
}

// Have again chosen not to use perfect forwarding here for better error
// message
void StringIndexes::insert(StringIndex si)
{
  // Unsure of evaluation order guarantees here (with or without c++17 changes).
  // https://en.cppreference.com/w/cpp/language/eval_order - Rules - point 15
  // means I think this would be unspecified so we need an additional line

  // Insert in sorted position if not already there
  const auto pos = std::lower_bound(
        stringindexes_.begin(), stringindexes_.end(), si);
  if (pos == stringindexes_.end() || *pos != si)
    stringindexes_.insert(pos, std::move(si));
  // Pairwise iteration for assert, not finding anything
//  std::adjacent_find(this->begin(), this->end(),
//                     [] (const auto& i1, const auto& i2)
//  {
//    JR_ASSERT(i1 != i2, "There should never be duplicates in "
//              "stringindexes_ but there are duplicate {}", i1
//              );
//    return false;
//  });
}

namespace
{
template <class T1, class T2>
__attribute__((__noinline__)) void insertion_f(T1& stringindexes_, const T2& stringindexes)
{
  stringindexes_.insert(stringindexes_.end(), stringindexes.begin(),
                        stringindexes.end());
}

template <class Iter>
__attribute__((__noinline__)) void merger_f(Iter first, Iter middle, Iter last)
{
  std::inplace_merge(first,
                     middle,
                     last);
}
}

void StringIndexes::concat(const StringIndexes& stringindexes)
{
  JR_ASSERT(this->grid_ == stringindexes.grid_,
            "Cannot concatenate StringIndexes from different grids");
  // stringindexes_.reserve(this->size() + stringindexes.size());
  // Iterator invalidation with vector underneath, take care
//  const auto old_size = static_cast<std::ptrdiff_t>(this->size());
  insertion_f(stringindexes_, stringindexes);
//  stringindexes_.insert(stringindexes_.end(), stringindexes.begin(),
//                        stringindexes.end());
//  merger_f(this->begin(), std::next(this->begin(), old_size), this->end());
//  std::inplace_merge(this->begin(),
//                     std::next(this->begin(), old_size),
//                     this->end());
//  JR_ASSERT(std::is_sorted(stringindexes_.begin(), stringindexes_.end()));
  // std::inplace_merge(this->begin());
  //  for (const auto& si: stringindexes)
  //  {
  //    this->insert(si);
  //  }
}

void StringIndexes::sort()
{
  std::sort(stringindexes_.begin(), stringindexes_.end());
}

void StringIndexes::unique()
{
  std::unique(stringindexes_.begin(), stringindexes_.end());
}

std::vector<std::string> StringIndexes::words() const
{
  std::vector<std::string> words;
  words.reserve(stringindexes_.size());
  for (const auto& p: stringindexes_)
    words.emplace_back(p.string());
  return words;
}

// Returns vector of pairs, where first is the word, and second the grid
// (multi line)
std::vector<std::pair<std::string, std::string>>
StringIndexes::to_grid_strings() const
{
  std::vector<std::pair<std::string, std::string>> string_index_printable;
  for (const auto& si: stringindexes_)
  {
    string_index_printable.push_back(
    {si.string(), stringindex_to_grid_string(grid_, si)});
  }
  return string_index_printable;
}

StringIndexes::StringIndexes(Grid grid)
  : grid_(std::move(grid))
  , stringindexes_()
{}

} // namespace wordsearch_solver
