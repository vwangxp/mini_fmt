#pragma once
#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace mini_fmt {

// ============================================================================
// 1. 核心类与模板的前置声明 (Forward Declarations)
// ============================================================================

template <typename T>
class basic_buffer;

template <typename T, std::size_t SIZE = 512>
class basic_memory_buffer;

using string_buffer = basic_memory_buffer<char>;

template <typename T>
class basic_buffer_appender;

using buffer_appender = basic_buffer_appender<char>;

class parse_context;

// formatter<T> 的默认参数在此处统一指定，后续定义无需再写 = char
template <typename T, typename Char = char>
struct formatter;

class format_arg;
class format_args;

namespace detail {

// 类型标记枚举 (Tagged Union 辨识符)
enum class type : std::uint8_t {
  none_type,
  int_type,
  uint_type,
  long_long_type,
  ulong_long_type,
  bool_type,
  char_type,
  float_type,
  double_type,
  string_type,
  pointer_type,
  custom_type  // 用户自定义类型擦除句柄
};

}  // namespace detail

}  // namespace mini_fmt