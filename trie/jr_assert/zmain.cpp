#include <algorithm>
#include <bits/c++config.h> // Remove this, coc.nvim suggests it
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fmt/core.h>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <iterator>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "prettyprint.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
// /usr/local/jr_assert/include/jr_assert.h
#include "jr_assert/jr_assert.h"

struct NoCopyOrMove
{
  NoCopyOrMove(const NoCopyOrMove&) = delete;
  NoCopyOrMove(NoCopyOrMove&&) = delete;
  NoCopyOrMove& operator=(const NoCopyOrMove&) = delete;
  NoCopyOrMove& operator=(NoCopyOrMove&&) = delete;
  NoCopyOrMove()
  {
    std::puts("NoCopyOrMove constructed");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  friend std::ostream& operator<<(std::ostream& os, const NoCopyOrMove&)
  {
    return os << "See the no copy or move!";
  }
};

template <>
struct fmt::formatter<NoCopyOrMove> {
  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && *it != '}')
      throw format_error("invalid format");
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const NoCopyOrMove&, FormatContext& ctx) {
    // ctx.out() is an output iterator to write to.
    return format_to(
        ctx.out(),
        "See the no copy or move");
  }
};

std::vector<unsigned long> f()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  return {3, 4, 5};
}

using namespace std::literals;

// #define JR_ASSERT_LEVEL JR_ASSERT_LEVEL_ON

// constexpr auto ff()
// {
// #ifdef JR_ASSERT
// #undef JR_ASSERT_LEVEL
// #endif
// #define JR_ASSERT_LEVEL 0
// }

// template<class... Args>
// void handle(const char* expr, const jr_assert::source_location& src_loc,
    // Args&&... args)
// {

// int main()
// {
  // std::string s{"A string!"};
  // auto f = [] () { return 42; };
  // int i = 69;
  // auto* p = &i;

  // // fmt::print("Should be: hi there {} {} {}\n", f(), s, NoCopyOrMove{});

  // fmt::print("Now true assert\n");
  // JR_ASSERT(false, "Should be nothing: hi there {} {} {}\n", f(), s, NoCopyOrMove{});
  // fmt::print("--\n");
  // // JR_ASSERT2(p, "Should be: hi there {} {} {}", f(), s, NoCopyOrMove{});
  // // JR_ASSERT(p);
  // // JR_ASSERT(false, "Should be nothing: hi there {} {} {}\n", f(), s, NoCopyOrMove{});
  // // JR_ASSERT(!p);
  // JR_UNREACHABLE();
  // // fmt::print("Now false assert\n");
  // // JR_ASSERT2(false);
/* } */

#include <string>
#include <fmt/format.h>
#include <jr_assert/jr_assert.h>

int main()
{
  std::string s{"Some text"};
  auto f = []() { return 42; };
  JR_ASSERT(f() == 42);
  JR_ASSERT(1 + 1 == 2, "Should not fire: {} {}", s, f());
  JR_ASSERT(1 + 1 != 2, "Should fire: {} {}", s, f());
}
