#pragma once

#include <charconv>
#include <chrono>
#include <cstddef>
#include <string_view>
#include <system_error>

#include "appender.h"
#include "fmtcore.h"

namespace mini_fmt {

enum class align_type { none, left, right, center };
enum class presentation {
  default_type,
  dec,
  hex,
  hex_upper,
  oct,
  bin,
  string,
  bool_zh,
  bool_en
};

struct format_specs {
  char fill = ' ';
  align_type align = align_type::none;
  std::size_t width = 0;
  int precision = -1;  // -1 表示未指定精度
  presentation type = presentation::default_type;
};

// 统一的解析器：在一个地方解析 [width][.precision][type]
constexpr const char* parse_format_specs(const char* begin, const char* end,
                                         format_specs& specs) {
  auto it = begin;
  if (it == end || *it == '}' || *it == ':') return it;

  // 1. 解析 fill 和 algin
  if (it + 1 < end &&
      (*(it + 1) == '<' || *(it + 1) == '>' || *(it + 1) == '^')) {
    specs.fill = *it;
    ++it;
  }
  if (it < end) {
    if (*it == '<') {
      specs.align = align_type::left;
      ++it;
    } else if (*it == '>') {
      specs.align = align_type::right;
      ++it;
    } else if (*it == '^') {
      specs.align = align_type::center;
      ++it;
    }
  }

  // 2. 解析 width (如 10)
  std::size_t w = 0;
  while (it != end && *it >= '0' && *it <= '9') {
    w = w * 10 + (*it - '0');
    ++it;
  }
  specs.width = w;

  // 3. 解析 .precision (如 .2)
  if (it != end && *it == '.') {
    ++it;
    int p = 0;
    while (it != end && *it >= '0' && *it <= '9') {
      p = p * 10 + (*it - '0');
      ++it;
    }
    specs.precision = p;
  }

  // 4. 解析 type (如 x, X, d, z, e)
  if (it != end && *it != '}' && *it != ':') {
    switch (*it) {
      case 'd':
        specs.type = presentation::dec;
        break;
      case 'x':
        specs.type = presentation::hex;
        break;
      case 'X':
        specs.type = presentation::hex_upper;
        break;
      case 'z':
        specs.type = presentation::bool_zh;
        break;
      case 'e':
        specs.type = presentation::bool_en;
        break;
      default:
        break;
    }
    ++it;
  }

  return it;
}

// 统一对齐渲染入口：把格式化好的原始内容 content，按照 specs 规则写入 out
inline void write_padded(std::string_view content, const format_specs& specs,
                         buffer_appender& out) {
  // 如果内容长度已经达到或超过指定 width，直接全量输出，无需填充
  if (content.length() >= specs.width) {
    for (char c : content) *out++ = c;
    return;
  }

  std::size_t total_padding = specs.width - content.length();
  std::size_t left_padding = 0;
  std::size_t right_padding = 0;

  // 默认对齐规则：如果未显式指定，文本/布尔默认左对齐，数值默认右对齐
  align_type actual_align = specs.align;
  if (actual_align == align_type::none) {
    actual_align = align_type::
        left;  // 默认左对齐，如果数值类需要默认右对齐可由数值格式化器传入
               // right
  }

  // 根据对齐类型计算左右填充长度
  switch (actual_align) {
    case align_type::left:
      left_padding = 0;
      right_padding = total_padding;
      break;
    case align_type::right:
      left_padding = total_padding;
      right_padding = 0;
      break;
    case align_type::center:
      left_padding = total_padding / 2;
      right_padding = total_padding - left_padding;
      break;
    default:
      break;
  }

  // 1. 追加左侧填充字符
  for (std::size_t i = 0; i < left_padding; ++i) *out++ = specs.fill;

  // 2. 追加实际文本内容
  for (char c : content) *out++ = c;

  // 3. 追加右侧填充字符
  for (std::size_t i = 0; i < right_padding; ++i) *out++ = specs.fill;
}

class parse_context {
 public:
  constexpr explicit parse_context(std::string_view fmt) noexcept : fmt_(fmt) {}

  constexpr auto begin() const noexcept { return fmt_.begin(); }
  constexpr auto end() const noexcept { return fmt_.end(); }

 private:
  std::string_view fmt_;
};

// formatter<T> 主模板 (默认删除，未特化类型引发编译期推导错误)
template <typename T, typename Char>
struct formatter {
  formatter() = delete;
};

// ============================================================================
// 通用数字格式化器辅助模板 (整数类型)
// ============================================================================
template <typename T>
struct integer_formatter {
  format_specs specs;

  constexpr auto parse(parse_context& ctx) {
    return parse_format_specs(ctx.begin(), ctx.end(), specs);
  }

  auto format(T val, buffer_appender& out) const {
    char buf[64];
    int base = 10;
    if (specs.type == presentation::hex ||
        specs.type == presentation::hex_upper)
      base = 16;
    else if (specs.type == presentation::oct)
      base = 8;
    else if (specs.type == presentation::bin)
      base = 2;
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), val, base);

    // std::string res_str;
    std::string_view res_view(buf, static_cast<std::size_t>(ptr - buf));

    if (specs.type == presentation::hex_upper) {
      for (char* p = buf; p != ptr; ++p) {
        if (*p >= 'a' && *p <= 'z') {
          *p -= 32;
        }
      }
    }

    // 数值类型未显式指定对齐时，默认右对齐
    format_specs num_specs = specs;
    if (num_specs.align == align_type::none) {
      num_specs.align = align_type::right;
    }

    // 统一委托对齐渲染
    write_padded(res_view, num_specs, out);

    return out;
  }
};

// 各种整数类型的特化继承
template <>
struct formatter<int> : integer_formatter<int> {};
template <>
struct formatter<unsigned> : integer_formatter<unsigned> {};
template <>
struct formatter<long long> : integer_formatter<long long> {};
template <>
struct formatter<unsigned long long> : integer_formatter<unsigned long long> {};

// ============================================================================
// 浮点数格式化器 (float & double)
// ============================================================================
template <typename T>
struct float_formatter {
  format_specs specs;

  constexpr auto parse(parse_context& ctx) {
    return parse_format_specs(ctx.begin(), ctx.end(), specs);
  }

  auto format(T val, buffer_appender& out) const {
    char buf[128];
    int len = 0;

    // 1. 根据 specs.precision 构造 snprintf 格式化字符串
    if (specs.precision >= 0) {
      // 指定精度 (例如 {:.2f} -> "%.2f")
      len = std::snprintf(buf, sizeof(buf), "%.*f", specs.precision,
                          static_cast<double>(val));
    } else {
      // 默认精度
      len = std::snprintf(buf, sizeof(buf), "%f", static_cast<double>(val));
    }

    std::string_view res_str(buf, len > 0 ? len : 0);

    // 2. 浮点数未显式指定对齐时，遵循数值类型的默认习惯：右对齐
    format_specs num_specs = specs;
    if (num_specs.align == align_type::none) {
      num_specs.align = align_type::right;
    }

    // 3. 统一委托给 write_padded 进行填充和对齐渲染
    write_padded(res_str, num_specs, out);
    return out;
  }
};

template <>
struct formatter<float> : float_formatter<float> {};
template <>
struct formatter<double> : float_formatter<double> {};

// ============================================================================
// 指针格式化器 (const void*)
// ============================================================================
struct pointer_formatter {
  format_specs specs;

  constexpr auto parse(parse_context& ctx) {
    return parse_format_specs(ctx.begin(), ctx.end(), specs);
  }

  auto format(const void* ptr, buffer_appender& out) const {
    if (ptr == nullptr) {
      write_padded("0x0", specs, out);
      return out;
    }

    // 将指针地址转换为无符号整数 uintptr_t
    auto addr = reinterpret_cast<std::uintptr_t>(ptr);

    char buf[32] = "0x";
    // 从 buf + 2 开始写入 16 进制字符串
    auto [p, ec] = std::to_chars(buf + 2, buf + sizeof(buf), addr, 16);

    std::string_view res_str(buf, static_cast<std::size_t>(p - buf));

    // 指针类型默认右对齐
    format_specs ptr_specs = specs;
    if (ptr_specs.align == align_type::none) {
      ptr_specs.align = align_type::right;
    }

    // 委托给 write_padded 进行统一的宽度与对齐处理
    write_padded(res_str, ptr_specs, out);
    return out;
  }
};

// 2. 注册 const void* 与 void* 的 formatter 特化
template <>
struct formatter<const void*> : public pointer_formatter {
  auto format(const void* ptr, buffer_appender& out) const {
    return pointer_formatter::format(ptr, out);
  }
};

template <>
struct formatter<void*> : public pointer_formatter {
  auto format(void* ptr, buffer_appender& out) const {
    return pointer_formatter::format(ptr, out);
  }
};

// 允许任意类型指针 (T*) 隐式匹配/特化
template <typename T>
struct formatter<T*> : public pointer_formatter {
  auto format(T* ptr, buffer_appender& out) const {
    return pointer_formatter::format(static_cast<const void*>(ptr), out);
  }
};

// ============================================================================
// 字符串与基础类型格式化器
// ============================================================================
template <>
struct formatter<std::string_view> {
  format_specs specs;

  constexpr auto parse(parse_context& ctx) {
    auto end = parse_format_specs(ctx.begin(), ctx.end(), specs);
    return end;
  }

  auto format(std::string_view val, buffer_appender& out) const {
    if (specs.precision >= 0 && static_cast<std::size_t>(specs.precision)) {
      val = val.substr(0, specs.precision);
    };
    write_padded(val, specs, out);
    return out;
  }
};

template <>
struct formatter<const char*> : formatter<std::string_view> {
  auto format(const char* val, buffer_appender& out) const {
    return formatter<std::string_view>::format(
        val ? std::string_view(val) : std::string_view("(null)"), out);
  }
};

template <>
struct formatter<std::string> : public formatter<std::string_view> {
  auto format(const std::string& val, buffer_appender& out) const {
    return formatter<std::string_view>::format(val, out);
  }
};

template <>
struct formatter<char> {
  constexpr auto parse(parse_context& ctx) { return ctx.begin(); }

  auto format(char val, buffer_appender& out) const {
    *out++ = val;
    return out;
  }
};

template <>
struct formatter<bool> {
  format_specs specs;

  constexpr auto parse(parse_context& ctx) {
    return parse_format_specs(ctx.begin(), ctx.end(), specs);
  }

  auto format(bool val, buffer_appender& out) const {
    std::string_view str = val ? "true" : "false";
    if (specs.type == presentation::bool_zh)
      str = val ? "男" : "女";
    else if (specs.type == presentation::bool_en)
      str = val ? "Male" : "Female";

    write_padded(str, specs, out);
    return out;
  }
};

// 辅助模板：根据 std::ratio 获取时间单位后缀字符串（如 ms, s, min）
template <typename Period>
struct ratio_suffix {
  static constexpr std::string_view value() { return "s"; }  // 默认单位为秒
};

template <>
struct ratio_suffix<std::milli> {
  static constexpr std::string_view value() { return "ms"; };
};
template <>
struct ratio_suffix<std::micro> {
  static constexpr std::string_view value() { return "us"; };
};
template <>
struct ratio_suffix<std::nano> {
  static constexpr std::string_view value() { return "ns"; };
};
template <>
struct ratio_suffix<std::ratio<60>> {
  static constexpr std::string_view value() { return "min"; };
};
template <>
struct ratio_suffix<std::ratio<3600>> {
  static constexpr std::string_view value() { return "h"; };
};

// std::chrono::duration 泛型特化
template <typename Rep, typename Period>
struct formatter<std::chrono::duration<Rep, Period>> {
  // 组合数值（整型或浮点型）格式化器
  format_specs specs_;
  formatter<Rep> val_fmt_;

  constexpr auto parse(parse_context& ctx) {
    // 将说明符解析（如宽度、对齐）透传给底层数值解析器
    auto it = parse_format_specs(ctx.begin(), ctx.end(), specs_);
    return it;
  }

  // 注意：必须为 const 方法，且接收 buffer_appender&
  auto format(const std::chrono::duration<Rep, Period>& dur,
              buffer_appender& out) const {
    // 2. 先把“数值 + 单位”渲染到内部临时 memory_buffer 中
    basic_memory_buffer<char> temp_buf;
    buffer_appender temp_out(temp_buf);

    // 格式化数值部分 (不带宽度限制)
    val_fmt_.format(dur.count(), temp_out);

    // 追加单位后缀
    constexpr auto suffix = ratio_suffix<typename Period::type>::value();
    for (char c : suffix) {
      *temp_out++ = c;
    }

    // 3. 将整个“1500ms”作为一个整体，交给 write_padded 统一进行对齐和宽度填充
    std::string_view content(temp_buf.data(), temp_buf.size());
    write_padded(content, specs_, out);

    return out;
  }
};

// 2. 为 std::vector<T> 编写容器特化 (如果库暂不支持，需在测试中先注释)
template <typename R>
  requires std::ranges::input_range<R> &&
           (!std::convertible_to<R, std::string_view>)
struct formatter<R> {
  using element_type = std::ranges::range_value_t<R>;
  formatter<element_type> elem_fmt_;
  constexpr auto parse(parse_context& ctx) { return elem_fmt_.parse(ctx); }

  auto format(const R& range, buffer_appender& out) const {
    // 格式化 vector 逻辑
    *out++ = '[';

    bool first = true;
    for (const auto& elem : range) {
      if (!first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;

      // 借用 elem_fmt_ 渲染单个元素
      elem_fmt_.format(elem, out);
    }

    *out++ = ']';
    return out;
  }
};

// / C++20 Concept: 匹配任意枚举类型
template <typename T>
  requires std::is_enum_v<T>
struct formatter<T> {
  using underlying_type = std::underlying_type_t<T>;
  integer_formatter<underlying_type> int_fmt_;

  constexpr auto parse(parse_context& ctx) { return int_fmt_.parse(ctx); }

  auto format(T val, buffer_appender& out) const {
    return int_fmt_.format(static_cast<underlying_type>(val), out);
  }
};

}  // namespace mini_fmt