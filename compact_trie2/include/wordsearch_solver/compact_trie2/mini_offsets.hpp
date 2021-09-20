#ifndef COMPACT_TRIE2_MINI_OFFSETS_HPP
#define COMPACT_TRIE2_MINI_OFFSETS_HPP

#include <range/v3/iterator/basic_iterator.hpp>
#include <range/v3/range/access.hpp>
#include <range/v3/view/facade.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

namespace compact_trie2 {

/** 2 byte wide view onto the offsets of child nodes, relative to the
 * CompactTrie2::next_row_offset().
 */
template <class Rng>
class MiniOffsets : public ranges::view_facade<MiniOffsets<Rng>> {
public:
  friend ranges::range_access;
  MiniOffsets() = default;

  struct cursor {
    friend ranges::range_access;
    using I = ranges::iterator_t<Rng>;
    struct mixin : ranges::basic_mixin<cursor> {
      mixin() = default;
      using ranges::basic_mixin<cursor>::basic_mixin;
      explicit mixin(I it) : mixin{cursor{std::forward<I>(it)}} {}
      I base() const { return this->get().it_; }
      void write(const std::uint16_t val) {
        // This assert is now trivially fulfilled
        assert(val < (1 << 2 * 8));
        // fmt::print("Write\n");
        std::memcpy(&*this->get().it_, &val, 2);
      }
    };

    I it_ = I();

    explicit cursor(I it) : it_(std::forward<I>(it)) {}
    void next() { ranges::advance(it_, 2); }
    std::uint16_t read() const {
      std::uint16_t n{};
      std::memcpy(&n, &*it_, 2);
      // fmt::print("Read\n");
      return n;
    }
    bool equal(cursor const& that) const { return it_ == that.it_; }
    void prev() { ranges::advance(it_, -2); }
    void advance(ranges::iter_difference_t<I> n) {
      ranges::advance(it_, 2 * n);
    }
    auto distance_to(cursor const& that) const {
      assert((that.it_ - it_) % 2 == 0);
      return (that.it_ - it_) / 2;
    }

  public:
    constexpr cursor() = default;
  };

  using Iterator = cursor;

  auto begin_cursor() const { return Iterator{ranges::begin(rng_)}; }
  auto end_cursor() const { return Iterator{ranges::end(rng_)}; }
  explicit MiniOffsets(Rng&& rng) : rng_(std::forward<Rng>(rng)) {}

  Rng rng_;
};

template <class Rng> MiniOffsets<Rng> make_mini_offsets(Rng&& rng) {
  return MiniOffsets<Rng>{std::forward<Rng>(rng)};
}

} // namespace compact_trie2

#endif // COMPACT_TRIE2_MINI_OFFSETS_HPP
