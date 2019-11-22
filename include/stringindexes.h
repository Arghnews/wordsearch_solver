#ifndef STRINGINDEXES_H
#define STRINGINDEXES_H

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "fmt/format.h"

#include "wordsearch_solver_defs.h"

#include <boost/iterator/iterator_facade.hpp>

namespace wordsearch_solver
{

class StringIndex
{
public:
  StringIndex(std::string string, Indexes indexes);
  StringIndex(std::string string,
              Indexes indexes, const Grid& grid);
  std::size_t size() const;
  const std::string& string() const;
  const Indexes& indexes() const;
private:
  std::string string_;
  Indexes indexes_;

  // TODO: rest of these ops/mixin for them if can get it to work
  // Or just get lazy, there must be a boost CRTP thingy that does this
  friend bool operator<(const StringIndex& si1, const StringIndex& si2);
  friend bool operator==(const StringIndex& si1, const StringIndex& si2);
  friend bool operator!=(const StringIndex& si1, const StringIndex& si2);
};

template <class Value>
class StringIndexIter
  : public boost::iterator_facade<
        StringIndexIter<Value>
      , Value
      , std::random_access_iterator_tag
    >
{
public:
  StringIndexIter()
    : node_(nullptr) {}

  explicit StringIndexIter(Value* p)
    : node_(p) {}

  template <class OtherValue,
            typename =
            std::enable_if_t<std::is_convertible_v<OtherValue*, Value*>>>
  StringIndexIter(StringIndexIter<OtherValue> const& other)
    : node_(other.node_) {}

private:
  friend class boost::iterator_core_access;
  template <class> friend class StringIndexIter;

  template <class OtherValue>
  bool equal(StringIndexIter<OtherValue> const& other) const
  {
    return this->node_ == other.node_;
  }

  template <class OtherValue>
  typename StringIndexIter::difference_type
  distance_to(const StringIndexIter<OtherValue>& other) const
  {
    return std::distance(node_, other.node_);
  }

  void advance(typename StringIndexIter::difference_type n)
  {
    std::advance(node_, n);
  }

  void increment()
  { ++node_; }

  void decrement()
  { --node_; }

  Value& dereference() const
  { return *node_; }

  Value* node_;
};

using StringIndexIterator = StringIndexIter<StringIndex>;
using StringIndexConstIterator = StringIndexIter<const StringIndex>;

// Possible TODO: rewrite with iterators, see if neater
class StringIndexes
{
public:
  using iterator = StringIndexIterator;
  using const_iterator = StringIndexConstIterator;

  // Have again chosen not to use perfect forwarding here for better error
  // message
  void insert(const StringIndex& si);
  void insert(StringIndex&& si);

  // TODO: unsure on this as the way to do it?
  void concat(const StringIndexes& stringindexes);

  std::vector<std::string> words() const;

  void erase(StringIndexIterator i1, StringIndexConstIterator i2)
  {
//    stringindexes_.erase();
    (void) i1;
    (void) i2;
  }

  // Solution for now to remove words from this
  template<class Pred>
  void filter(Pred&& pred)
  {
    stringindexes_.erase(
          std::remove_if(stringindexes_.begin(), stringindexes_.end(), pred)
          , stringindexes_.end());
  }

  // Returns vector of pairs, where first is the word, and second the grid
  // (multi line)
  std::vector<std::pair<std::string, std::string>> to_grid_strings() const;

  StringIndexes(Grid grid);

  void sort();
  void unique();

  auto begin()
  {
    return StringIndexIterator(&*stringindexes_.begin());
  }
  auto begin() const
  {
    return StringIndexConstIterator(&*stringindexes_.begin());
  }

  auto end()
  {
    // Assuming &*stringindexes_.end() is UB?
    return StringIndexIterator(
          &*stringindexes_.begin() + stringindexes_.size());
  }
  auto end() const
  {
    return StringIndexConstIterator(
          &*stringindexes_.begin() + stringindexes_.size());
  }

  std::size_t size() const
  {
    return stringindexes_.size();
//    return static_cast<std::size_t>(std::distance(this->begin(), this->end()));
  }

private:
  Grid grid_;
  std::vector<StringIndex> stringindexes_;
};

} // namespace wordsearch_solver

#endif // STRINGINDEXES_H
