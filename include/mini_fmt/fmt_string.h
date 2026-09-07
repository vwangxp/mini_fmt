#pragma once

#include <cstddef>
#include <stdexcept>
#include <string_view>

#include "fmtcore.h"

namespace mini_fmt {

template <typename... Args>
class basic_format_string {
 public:
  template <typename S>
    requires std::is_convertible_v<const S&, std::string_view>
  consteval basic_format_string(const S& str) : str_(str) {
    // 编译期校验格式说明符匹配
    std::size_t arg_count = 0;
    bool in_brace = false;

    for (std::size_t i = 0; i < str_.size(); ++i) {
      if (str_[i] == '{') {
        if (i + 1 < str_.size() && str_[i + 1] == '{') {
          ++i; // 转义 {{
        } else {
          in_brace = true;
          ++arg_count;
        }
      } else if (str_[i] == '}') {
        if (i + 1 < str_.size() && str_[i + 1] == '}') {
          ++i; // 转义 }}
        } else if (in_brace) {
          in_brace = false;
        }
      }
    }

    if (arg_count > sizeof...(Args)) {
      throw "format string references more arguments than supplied!";
    }
  }

  constexpr std::string_view get() const noexcept { return str_; }

 private:
  std::string_view str_;
};

template <typename... Args>
using format_string = basic_format_string<Args...>;

}  // namespace mini_fmt