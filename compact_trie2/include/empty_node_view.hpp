#ifndef EMPTYNODEVIEW_HPP
#define EMPTYNODEVIEW_HPP

#include "compact_trie2_iterator_typedefs.hpp"

#include <cstdint>
#include <ostream>
#include <vector>

//#include "prettyprint.hpp"
//#include <fmt/format.h>
//#include <fmt/ostream.h>

// Unsure if constexpr makes any difference to member functions here
template<class Iterator>
class EmptyNodeView_
{
  public:

  constexpr explicit EmptyNodeView_(Iterator it)
    : it_(it)
  {}

  constexpr auto base() const
  {
    return it_;
  }

  constexpr std::size_t size() const
  {
    return 1;
  }

  constexpr bool is_only_end_of_word() const
  {
    return true;
  }

  constexpr std::uint8_t data_size() const
  {
    // gcc complains that the iterator's operator* here isn't constexpr
    // clang doesn't
    // assert(*it_ == 0);
    return 0;
  }

  constexpr bool is_end_of_word() const
  {
    return true;
  }

  friend std::ostream& operator<<(std::ostream& os, const EmptyNodeView_&)
  {
    return os << "EmptyNode";
  }

  Iterator it_;
};

using EmptyNodeViewMut = EmptyNodeView_<DataIteratorMut>;
using EmptyNodeView = EmptyNodeView_<DataIterator>;

#endif // EMPTYNODEVIEW_HPP
