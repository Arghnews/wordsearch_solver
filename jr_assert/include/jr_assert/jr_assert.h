#ifndef JR_ASSERT_H
#define JR_ASSERT_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <utility>

#include <fmt/format.h>
#include <fmt/core.h>
#include "jr_assert/hedley/hedley.h"

#if (defined(__GNUC__) && ((__GNUC__ * 1000 + __GNUC_MINOR__ * 100) >= 4600)) || defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wvariadic-macros"
#endif

#if defined(__clang__)
  #pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

namespace jr_assert
{

struct source_location
{
  std::uint_least32_t line_;
  std::uint_least32_t column_;
  const char* file_name_;
  const char* function_name_;

  constexpr std::uint_least32_t line() const noexcept
  {
    return line_;
  }
  constexpr std::uint_least32_t column() const noexcept
  {
    return column_;
  }
  constexpr const char* file_name() const noexcept
  {
    return file_name_;
  }
  constexpr const char* function_name() const noexcept
  {
    return function_name_;
  }

  // source_location() = default;
  // source_location(const source_location&) = default;
  // source_location& operator=(const source_location&) = default;
};

static_assert(std::is_nothrow_move_constructible_v<source_location>);
static_assert(std::is_nothrow_move_assignable_v<source_location>);
static_assert(std::is_nothrow_swappable_v<source_location>);

static_assert(std::is_nothrow_default_constructible_v<source_location>);
static_assert(std::is_nothrow_constructible_v<source_location>);
static_assert(std::is_nothrow_copy_constructible_v<source_location>);
static_assert(std::is_nothrow_copy_assignable_v<source_location>);
static_assert(std::is_nothrow_destructible_v<source_location>);

#define SOURCE_LOCATION_CURRENT jr_assert::source_location{ \
  __LINE__, 0, __FILE__, __FUNCTION__}

template<class... Args>
static void handle(
    const char* expression,
    const source_location& src,
    Args&&... args)
{
  fmt::memory_buffer b;
  fmt::format_to(b, FMT_STRING("{}:{}: Assert failed: `{}`"),
      src.file_name(), src.line(), expression);
  if constexpr (sizeof...(args) > 0)
  {
    b.push_back(' ');
    fmt::format_to(b, std::forward<Args>(args)...);
  }
  b.push_back('\n');
  std::copy(b.begin(), b.end(), std::ostream_iterator<char>(std::cerr));
}

} //namespace jr_assert

#define JR_ASSERT_DISABLED_IMPL(Expr, ...) (void)(true ? (void)0 : ((void)(Expr)))

#define JR_ASSERT_ENABLED_IMPL(Expr, ...) \
  static_cast<bool>((Expr)) ? void() : \
  (jr_assert::handle(#Expr, SOURCE_LOCATION_CURRENT, ##__VA_ARGS__), \
  std::abort(), void())

#if defined(NDEBUG) || defined(JR_ASSERT_DISABLE)
  #define JR_ASSERT(Expr, ...) JR_ASSERT_DISABLED_IMPL(Expr, ##__VA_ARGS__)
#else
  #define JR_ASSERT(Expr, ...) JR_ASSERT_ENABLED_IMPL(Expr, ##__VA_ARGS__)
#endif

#define JR_UNREACHABLE() HEDLEY_UNREACHABLE()

#define JR_ASSERT_ALWAYS_ENABLED(Expr, ...) \
  JR_ASSERT_ENABLED_IMPL(Expr, ##__VA_ARGS__)

#if (defined(__GNUC__) && ((__GNUC__ * 1000 + __GNUC_MINOR__ * 100) >= 4600)) || defined(__clang__)
  #pragma GCC diagnostic pop
#endif

#endif
