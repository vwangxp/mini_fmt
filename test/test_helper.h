#pragma once

#include "mini_fmt/mini_fmt.h"

namespace mini_fmt::test {

template <typename Formatter, typename T>
std::string test_format(Formatter& fmt, const T& val) {
  string_buffer buf;
  buffer_appender out(buf);
  fmt.format(val, out);
  return std::string(buf.data(), buf.size());
}

}