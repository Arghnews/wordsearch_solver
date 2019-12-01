#ifndef JR_ASSERT_H
#define JR_ASSERT_H

#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <cstdlib>
#include <type_traits>
#include <utility>

/*
 * Simple improved assert that prints file location and expression that failed
 * and logs messages to spdlog::error. Requires spdlog and fmt libraries.
 * Potential improvements:
 * Log to different loggers, more configurability overall
 * Have not tried combinations of using spdlog's bundled fmt, works with
 * separate fmt(6.0.0) and spdlog(1.4.2).
 * Feels a bit messy/hacky but it seems to work.
 */

// https://stackoverflow.com/a/28166605/8594193
#if defined(__GNUC__) && !defined(__clang__)
#    define JR_GCC
#endif

/*
 * Suppress warnings (on gcc and clang) about the use of the gnu extension to
 * allow zero args to be passed to a variadic macro even when using -pedantic.
 */

#ifdef JR_GCC
  // Yuck.
  #pragma GCC system_header
#elif defined (__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

namespace jr_assert
{

template <class... Args>
[[noreturn]]
void assert_after_func(const char* file, const int line, const char* func,
                       const char* expression,
                       Args&&... args)
                       // [[maybe_unused]] const AssertConfig& config,
{
  // fmt::print("Assert config IS called!\n");
  fmt::memory_buffer b;
  fmt::format_to(b, FMT_STRING("{}:{} in: {} - Assert failed `{}`"), file,
      line, func, expression);
  if constexpr (sizeof...(args) > 0)
  {
    b.push_back(' ');
    fmt::format_to(b, std::forward<Args>(args)...);
  }
  std::cerr << fmt::to_string(b) << "\n";
  // spdlog::error(std::string_view{b.data(), b.size()});
  // Flush before abort
  // https://github.com/gabime/spdlog/wiki/7.-Flush-policy#manual-flush
  std::abort();
}

}

// Wanted to add static_assert(!std::is_array_v<decltype(assertion)>);
// To catch evaluating string literals by mistake but then get lambda in
// unevaluated context error

#ifdef JR_DISABLE_ASSERTS
  #define JR_ASSERT(assertion, ...)                                            \
  do                                                                           \
  {                                                                            \
    [] (...) {} (assertion, ##__VA_ARGS__);                                    \
  }                                                                            \
  while (0)
#else
  #define JR_ASSERT(assertion, ...)                                            \
  do                                                                           \
  {                                                                            \
    if (!(static_cast<bool>(assertion)))                                       \
    {                                                                          \
      jr_assert::assert_after_func(__FILE__, __LINE__, __func__,               \
                                #assertion, ##__VA_ARGS__);                    \
    }                                                                          \
  } while (0)
#endif

// TODO: look into this
#ifdef __clang__
  #pragma clang diagnostic pop
#endif

/*
 * struct AssertConfig
 * {
 * };
 *
 */
/*
 * template <class T, class... Args>
 * struct HeadOfParameterPackIsSameTypeAs : std::false_type {};
 *
 * template <class T, class... Args>
 * struct HeadOfParameterPackIsSameTypeAs<T, T, Args...> : std::true_type {};
 *
 * template <class... Args>
 * void assert_after_func_wrapper(const char* file, const int line,
 *                                const char* func, const char* expression,
 *                                Args&&... args)
 * {
 *   fmt::print("Wrapper called!\n");
 *   if constexpr (HeadOfParameterPackIsSameTypeAs<AssertConfig, Args...>())
 *   {
 *     fmt::print("First argument is AssertConfig\n");
 *     assert_after_func(file, line, func, expression,
 *         std::forward<Args>(args)...);
 *   } else
 *   {
 *     fmt::print("Normal version with default AssertConfig called\n");
 *     assert_after_func(file, line, func, expression, {},
 *         std::forward<Args>(args)...);
 *   }
 * }
 *
 */

#endif
