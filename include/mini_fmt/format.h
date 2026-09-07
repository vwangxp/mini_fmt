#pragma once

#include <cstddef>
#include <iostream>
#include <string_view>
#include <type_traits>

#include "appender.h"
#include "buffer.h"
#include "fmt_string.h"
#include "fmtargs.h"
#include "formatter.h"

namespace mini_fmt {

// C++20 type_identity 实现 (若编译器支持 C++20，可以直接使用
// std::type_identity_t)
template <typename T>
struct type_identity {
  using type = T;
};

template <typename T>
using type_identity_t = typename type_identity<T>::type;

namespace detail {

// 判断类型是否具备合法的 formatter 特化
template <typename T, typename = void>
struct is_formattable : std::false_type {};

template <typename T>
struct is_formattable<
    T, std::void_t<decltype(std::declval<formatter<T>>().parse(
                       std::declval<parse_context&>())),
                   decltype(std::declval<formatter<T>>().format(
                       std::declval<T>(), std::declval<buffer_appender&>()))>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_formattable_v = is_formattable<T>::value;

inline void vformat_to(buffer_appender& out, std::string_view fmt_str,
                       format_args args) {
  std::size_t arg_index = 0;
  std::size_t i = 0;
  const std::size_t len = fmt_str.size();

  while (i < len) {
    // 批量复制文本段
    std::size_t text_start = i;
    while (i < len && fmt_str[i] != '{' && fmt_str[i] != '}') {
      ++i;
    }
    if (i > text_start) {
      out.append(fmt_str.data() + text_start, i - text_start);
    }

    if (i >= len) break;

    char c = fmt_str[i];

    if (c == '{') {
      if (i + 1 < len && fmt_str[i + 1] == '{') {
        *out++ = '{';
        i += 2;
        continue;
      }

      std::size_t spec_start = i + 1;
      std::size_t spec_end = spec_start;
      while (spec_end < len && fmt_str[spec_end] != '}') {
        ++spec_end;
      }

      std::string_view spec = fmt_str.substr(spec_start, spec_end - spec_start);
      if (!spec.empty() && spec.front() == ':') {
        spec.remove_prefix(1);
      }

      format_arg arg = args.get(arg_index++);
      if (arg) {
        parse_context pctx(spec);
        arg.format_to(pctx, out);
      }

      i = (spec_end < len) ? spec_end + 1 : len;
    } else if (c == '}') {
      if (i + 1 < len && fmt_str[i + 1] == '}') {
        *out++ = '}';
        i += 2;
        continue;
      }
      ++i;
    }
  }
}

}  // namespace detail

// 1. vformat
template <typename... Args>
inline std::string vformat(std::string_view fmt, format_args args) {
  string_buffer buf;
  // Pre-reserve capacity based on format string length + estimated arg size
  buf.reserve(fmt.size() + args.size() * 16);
  buffer_appender appender(buf);
  detail::vformat_to(appender, fmt, args);
  return std::string(buf.data(), buf.size());
}

// 2. format：使用 type_identity_t 隔离 fmt 的模板推导
template <typename... Args>
[[gnu::always_inline]] inline std::string format(format_string<type_identity_t<Args>...> fmt,
                          const Args&... args) {
  format_arg_store<sizeof...(Args)> store{format_arg(args)...};
  format_args args_view(store.data, sizeof...(Args));
  return vformat(fmt.get(), args_view);
}

// Fast path: single integral argument - compile-time specialization
template <typename T>
  requires (std::is_integral_v<T> && !std::is_same_v<T, bool>)
[[gnu::always_inline]] inline std::string format(const char* fmt_str, T value) {
  string_buffer buf;
  std::string_view fmt_view(fmt_str);
  buf.reserve(fmt_view.size() + 20);
  buffer_appender out(buf);

  // Fast scan for simple "text{}" pattern
  const char* p = fmt_str;
  const char* start = p;
  while (*p && *p != '{') ++p;

  // Copy prefix
  if (p > start) {
    out.append(start, p - start);
  }

  // Format integer if we have a placeholder
  if (*p == '{' && *(p+1) == '}') {
    char int_buf[64];
    char* end_ptr;
    if constexpr (std::is_signed_v<T>) {
      end_ptr = detail::format_signed_lut_to_buf(value, int_buf);
    } else {
      end_ptr = detail::format_unsigned_lut_to_buf(value, int_buf);
    }
    out.append(int_buf, static_cast<std::size_t>(end_ptr - int_buf));

    // Copy suffix
    p += 2;
    while (*p) {
      *out++ = *p++;
    }

    return std::string(buf.data(), buf.size());
  }

  // Fallback to general path for complex formats
  format_arg_store<1> store{format_arg(value)};
  format_args args_view(store.data, 1);
  return vformat(fmt_view, args_view);
}

// 3. print：同理隔离推导，并显式指定 format<Args...>
template <typename... Args>
inline void print(format_string<type_identity_t<Args>...> fmt,
                  const Args&... args) {
  std::string result = format<Args...>(fmt, args...);
  std::cout << result;
}

}  // namespace mini_fmt