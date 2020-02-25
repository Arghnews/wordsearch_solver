#ifndef FULLNODEVIEW_H
#define FULLNODEVIEW_H

#include "mini_offsets.hpp"
#include "compact_trie2_iterator_typedefs.hpp"

#include <fmt/format.h>
#include <range/v3/iterator/access.hpp>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/transform.hpp>

#include <bitset>
#include <cstdint>
#include <cstring>
#include <ostream>
#include <type_traits>
#include <vector>

template<class Iterator>
class FullNodeView_
{
  public:

  explicit FullNodeView_(Iterator it)
    : it_(it)
  {}

  auto base() const
  {
    return it_;
  }

  // std::size_t size() const
  std::size_t size() const
  {
    return this->data_size() * 3 + 2;
    // return this->data_size() * 2 + 3;
    // return this->is_only_end_of_word() ? 1 : 2 * this->data_size() + 3;
  }

  bool is_only_end_of_word() const
  {
    return false;
    // return this->data_size() == 0;
  }

  std::uint8_t data_size() const
  {
    return *it_;
  }

  bool is_end_of_word() const
  {
    // fmt::print("THIS IS THE DROID YOU'RE LOOKING FOR\n");
    // fmt::print("it_ + 1: {:#08x}\n", uint(*(it_ + 1)));
    // fmt::print("it_ + 2: {:#08x}\n", uint(*(it_ + 2)));
    // fmt::print("it_ + 3: {:#08x}\n", uint(*(it_ + 3)));
    return *(it_ + 3) & 0x80;
  }

  std::uint_fast32_t next_row_offset() const
  {
    assert(!this->is_only_end_of_word());
    std::uint_fast32_t n{};
    std::memcpy(&n, &*(it_ + 1), 3);
    // fmt::print("n memcpyed {} {}\n", n, std::bitset<32>{n});
    // n &= 0x7f'ff'ff;
    n &= ~(1UL << 23);
    // fmt::print("n returned {} {}\n", n, std::bitset<32>{n});
    // Disable highest bit as it's the end_of_word bit
    return n;
  }

  auto data() const
  {
    const auto offset = it_ + 4;
    return ranges::subrange(offset, offset + this->data_size());
  }

  auto mini_offsets() const
  {
    assert(!this->is_only_end_of_word());
    const auto offset = it_ + 4 + this->data_size();
    assert(this->data_size() > 0);
    return make_mini_offsets(ranges::subrange(offset,
          offset + 2 * (this->data_size() - 1)));
  }

  // next_row_offset in bytes
  template<class = std::enable_if_t<
    std::is_convertible_v<
      std::add_pointer_t<ranges::iter_value_t<Iterator>>
      , void*>
    >>
  void set_next_row_offset(const std::uint_fast32_t next_row_offset)
  {
    auto it = it_ + 1;
    assert(next_row_offset < 1 << 23);
    // Mask off top byte and also end_of_word bit

    std::bitset<24> bits{next_row_offset};
    bits[23] = this->is_end_of_word();
    const auto n = bits.to_ulong();
    // next_row_offset |= this->is_end_of_word() << 7;
    // next_row_offset <<= 8;
    std::memcpy(&*it, &n, 3);
  }

  // template<class Range>
  // void set_mini_offsets(Range&& mini_offsets)
  // {
    // assert(!this->is_only_end_of_word());
    // assert(mini_offsets.size() == this->data_size() - 1);
    // auto offset = it_ + 4 + this->data_size();
    // // const auto offset_end = offset + this->data_size() - 1;
    // for (const auto val: mini_offsets)
    // {
      // *offset++ = val;
    // }
  // }

  friend std::ostream& operator<<(std::ostream& os, const FullNodeView_& nv)
  {
    auto to_uint = ranges::views::transform(
        [] (const auto val) { return static_cast<std::size_t>(val); });
    auto to_char = ranges::views::transform(
        [] (const auto val) { return static_cast<char>(val); });

    const auto size = nv.size();
    const bool is_end_of_word = nv.is_end_of_word();
    const auto next_row_offset = nv.next_row_offset();
    const auto data = nv.data() | to_char;

    // fmt::print(">>>>>>>>> About to print mini_offsets <<<<<<<<\n");
    const auto mini_offsets = nv.mini_offsets() | to_uint;
    // fmt::print("mini_offsets: {}\n", mini_offsets);

    // return os << fmt::format("FullNode: {{Size: {}, EndOfWord: {}, "
        // "next_row_offset: {}, data: {}, mini_offsets: {}}}",
        // size, is_end_of_word, next_row_offset, data, mini_offsets);
    return os << fmt::format("FullNode: {{Size: {}, EndOfWord: {}, "
        "next_row_offset: {}, data: {}, mini_offsets {}}}",
        size, is_end_of_word, next_row_offset, data, mini_offsets);
  }

  Iterator it_;
};

using FullNodeViewMut = FullNodeView_<DataIteratorMut>;
using FullNodeView = FullNodeView_<DataIterator>;

#endif // FULLNODEVIEW_H
