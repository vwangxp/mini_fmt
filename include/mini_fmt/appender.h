#pragma once

#include <cstddef>
#include <iterator>
#include <utility>

#include "buffer.h"
#include "fmtcore.h"

namespace mini_fmt {

template <typename T>
class basic_buffer_appender {
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using difference_type = std::ptrdiff_t;
  using pointer = void;
  using reference = void;

  explicit constexpr basic_buffer_appender(basic_buffer<T>& buf) noexcept
      : buf_(&buf) {}

  constexpr void append(const T* data, std::size_t count) {
    buf_->append(data, data + count);
  }

  [[nodiscard]] constexpr basic_buffer_appender& operator*() noexcept {
    return *this;
  }

  constexpr basic_buffer_appender& operator=(const T& value) {
    buf_->push_back(value);
    return *this;
  }

  constexpr basic_buffer_appender& operator=(T&& value) {
    buf_->push_back(std::move(value));
    return *this;
  }

  constexpr basic_buffer_appender& operator++() noexcept { return *this; }
  constexpr basic_buffer_appender operator++(int) noexcept { return *this; }

  [[nodiscard]] constexpr basic_buffer<T>& get_buffer() const noexcept {
    return *buf_;
  }

 private:
  basic_buffer<T>* buf_;
};

}  // namespace mini_fmt