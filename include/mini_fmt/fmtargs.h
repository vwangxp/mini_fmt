#pragma once

#include <concepts>
#include <cstddef>
#include <string_view>
#include <type_traits>

#include "fmtcore.h"
#include "formatter.h"

namespace mini_fmt {

class format_arg {
 public:
  using handle_func = void (*)(const void* value_ptr, parse_context&,
                               buffer_appender& out);

  struct handle {
    const void* value;
    handle_func format_fn;
  };

  constexpr format_arg() noexcept : type_(detail::type::none_type) {}

  constexpr format_arg(int v) noexcept : type_(detail::type::int_type) {
    value_.int_value = v;
  }

  constexpr format_arg(unsigned v) noexcept : type_(detail::type::uint_type) {
    value_.uint_value = v;
  }

  constexpr format_arg(long long v) noexcept
      : type_(detail::type::long_long_type) {
    value_.long_long_value = v;
  }

  constexpr format_arg(unsigned long long v) noexcept
      : type_(detail::type::ulong_long_type) {
    value_.ulong_long_value = v;
  }

  constexpr format_arg(bool v) noexcept : type_(detail::type::bool_type) {
    value_.bool_value = v;
  }

  constexpr format_arg(char v) noexcept : type_(detail::type::char_type) {
    value_.char_value = v;
  }

  constexpr format_arg(float v) noexcept : type_(detail::type::float_type) {
    value_.float_value = v;
  }

  constexpr format_arg(double v) noexcept : type_(detail::type::double_type) {
    value_.double_value = v;
  }

  constexpr format_arg(std::string_view v) noexcept
      : type_(detail::type::string_type) {
    value_.string_value = v;
  }

  constexpr format_arg(const void* v) noexcept
      : type_(detail::type::pointer_type) {
    value_.pointer_value = v;
  }

  constexpr format_arg(const char* v) noexcept
      : format_arg(std::string_view(v ? v : "(null)")) {}

  // 通用类型擦除构造句柄
  template <typename T>
    requires(!std::is_same_v<std::remove_cvref_t<T>, format_arg>)
  explicit constexpr format_arg(const T& val) noexcept
      : type_(detail::type::custom_type) {
    value_.handle_value.value = static_cast<const void*>(&val);
    // ✅ 修复：补全 parse_context& ctx 参数，并先调用 parse(ctx)
    value_.handle_value.format_fn = [](const void* ptr, parse_context& ctx,
                                       buffer_appender& out) {
      const T& typed_val = *static_cast<const T*>(ptr);
      formatter<T> fmt;
      fmt.parse(ctx);              // 1. 先解析说明符 (如 'x', 'X' 等)
      fmt.format(typed_val, out);  // 2. 再执行格式化输出
    };
  }

  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return type_ != detail::type::none_type;
  }

  [[nodiscard]] constexpr detail::type get_type() const noexcept {
    return type_;
  }

  template <typename Visitor>
  decltype(auto) visit(Visitor&& vis) const {
    switch (type_) {
      case detail::type::int_type:
        return vis(value_.int_value);
      case detail::type::uint_type:
        return vis(value_.uint_value);
      case detail::type::long_long_type:
        return vis(value_.long_long_value);
      case detail::type::ulong_long_type:
        return vis(value_.ulong_long_value);
      case detail::type::bool_type:
        return vis(value_.bool_value);
      case detail::type::char_type:
        return vis(value_.char_value);
      case detail::type::float_type:
        return vis(value_.float_value);
      case detail::type::double_type:
        return vis(value_.double_value);
      case detail::type::string_type:
        return vis(value_.string_value);
      case detail::type::pointer_type:
        return vis(value_.pointer_value);
      case detail::type::custom_type:
        return vis(value_.handle_value);
      default:
        return vis(nullptr);
    }
  }

  // Fast path for common types - avoid visit overhead
  [[gnu::always_inline]] inline void format_to(parse_context& pctx, buffer_appender& out) const {
    switch (type_) {
      case detail::type::int_type: {
        formatter<int> fmt;
        fmt.parse(pctx);
        fmt.format(value_.int_value, out);
        break;
      }
      case detail::type::string_type: {
        formatter<std::string_view> fmt;
        fmt.parse(pctx);
        fmt.format(value_.string_value, out);
        break;
      }
      default:
        // Fallback to visit for other types
        visit([&](auto&& val) {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, format_arg::handle>) {
            val.format_fn(val.value, pctx, out);
          } else if constexpr (!std::is_same_v<T, std::nullptr_t>) {
            formatter<T> fmt;
            fmt.parse(pctx);
            fmt.format(val, out);
          }
        });
        break;
    }
  }

 private:
  detail::type type_{detail::type::none_type};

  union value {
    int int_value;
    unsigned uint_value;
    long long long_long_value;
    unsigned long long ulong_long_value;
    bool bool_value;
    char char_value;
    float float_value;
    double double_value;
    std::string_view string_value;
    const void* pointer_value;
    handle handle_value;

    constexpr value() : pointer_value(nullptr) {}
  } value_;
};

class format_args {
 public:
  constexpr format_args() noexcept : args_(nullptr), size_(0) {}

  constexpr format_args(const format_arg* args, std::size_t count) noexcept
      : args_(args), size_(count) {}

  [[nodiscard]] constexpr format_arg get(std::size_t index) const noexcept {
    if (index < size_) {
      return args_[index];
    }
    return format_arg{};
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }

 private:
  const format_arg* args_;
  std::size_t size_;
};

template <std::size_t N>
struct format_arg_store {
  format_arg data[N == 0 ? 1 : N];
};

template <typename... Args>
constexpr auto make_format_args(const Args&... args) {
  format_arg_store<sizeof...(Args)> store{format_arg(args)...};
  return store;
}

}  // namespace mini_fmt