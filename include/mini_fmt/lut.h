#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace mini_fmt::detail {

// 预计算 00-99 的 2 位 ASCII 字符查找表 (200 字节)
inline constexpr char DIGITS_LUT[200] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
    '7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
    '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2',
    '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3',
    '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
    '4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5',
    '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6',
    '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
    '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8',
    '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9',
    '7', '9', '8', '9', '9'};

template <typename T>
  requires std::is_unsigned_v<T>
inline char* format_unsigned_lut_to_buf(T value, char* buf) {
  char temp[24];
  char* ptr = temp + sizeof(temp);

  while (value >= 100) {
    auto index = static_cast<unsigned>(value % 100) * 2;
    value /= 100;
    *--ptr = DIGITS_LUT[index + 1];
    *--ptr = DIGITS_LUT[index];
  }

  if (value < 10) {
    *--ptr = static_cast<char>('0' + value);
  } else {
    auto index = static_cast<unsigned>(value) * 2;
    *--ptr = DIGITS_LUT[index + 1];
    *--ptr = DIGITS_LUT[index];
  }

  // 拷贝至目标 buf
  std::size_t len = temp + sizeof(temp) - ptr;
  std::memcpy(buf, ptr, len);
  return buf + len;
}

template <typename T>
  requires std::is_signed_v<T>
inline char* format_signed_lut_to_buf(T value, char* buf) {
  using UnsignedT = std::make_unsigned_t<T>;
  UnsignedT abs_val;
  if (value < 0) {
    *buf++ = '-';
    abs_val = 0 - static_cast<UnsignedT>(value);
  } else {
    abs_val = static_cast<UnsignedT>(value);
  }
  return format_unsigned_lut_to_buf(abs_val, buf);
}

}  // namespace mini_fmt::detail