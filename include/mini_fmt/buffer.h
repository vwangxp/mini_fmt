#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <utility>

#include "fmtcore.h"

namespace mini_fmt {

template <typename T>
class basic_buffer {
 public:
  using value_type = T;
  using size_type = std::size_t;

  basic_buffer(const basic_buffer&) = delete;
  basic_buffer& operator=(const basic_buffer&) = delete;

  virtual ~basic_buffer() = default;

  constexpr T* data() noexcept { return ptr_; }
  constexpr const T* data() const noexcept { return ptr_; }

  constexpr size_type size() const noexcept { return size_; }
  constexpr size_type capacity() const noexcept { return capacity_; }

  constexpr void clear() noexcept { size_ = 0; }

  [[gnu::always_inline]] constexpr void push_back(const T& value) {
    try_reserve(size_ + 1);
    ptr_[size_++] = value;
  }

  [[gnu::always_inline]] constexpr void push_back(T&& value) {
    try_reserve(size_ + 1);
    ptr_[size_++] = std::move(value);
  }

  [[gnu::always_inline]] constexpr void append(const T* begin, const T* end) {
    size_type count = end - begin;
    size_type new_size = size_ + count;
    try_reserve(new_size);
    std::memcpy(ptr_ + size_, begin, count * sizeof(T));
    size_ = new_size;
  }

  constexpr void reserve(size_type new_capacity) {
    if (new_capacity > capacity_) {
      grow(new_capacity);
    }
  }

  // 迭代器支持
  constexpr T* begin() noexcept { return ptr_; }
  constexpr T* end() noexcept { return ptr_ + size_; }
  constexpr const T* begin() const noexcept { return ptr_; }
  constexpr const T* end() const noexcept { return ptr_ + size_; }

 protected:
  constexpr basic_buffer(T* ptr, size_type capacity) noexcept
      : ptr_(ptr), capacity_(capacity), size_(0) {}

  constexpr void set(T* buf_data, size_type buf_capacity) noexcept {
    ptr_ = buf_data;
    capacity_ = buf_capacity;
  }

  virtual void grow(size_type capacity) = 0;

 private:
  constexpr void try_reserve(size_type new_capacity) {
    if (new_capacity > capacity_) {
      grow(new_capacity);
    }
  }

  T* ptr_;
  size_type capacity_;
  size_type size_;
};

// 带 SBO 栈上缓冲区的实现类
template <typename T, std::size_t SIZE>
class basic_memory_buffer : public basic_buffer<T> {
 public:
  basic_memory_buffer() noexcept : basic_buffer<T>(storage_, SIZE) {}

  ~basic_memory_buffer() override {
    if (this->data() != storage_) {
      delete[] this->data();
    }
  }

  basic_memory_buffer(basic_memory_buffer&& other) noexcept
      : basic_buffer<T>(storage_, SIZE) {
    move_from(std::move(other));
  }

  basic_memory_buffer& operator=(basic_memory_buffer&& other) noexcept {
    if (this != &other) {
      if (this->data() != storage_) {
        delete[] this->data();
      }
      move_from(std::move(other));
    }
    return *this;
  }

 protected:
  void grow(std::size_t capacity) override {
    std::size_t new_cap = (this->capacity() == 0) ? SIZE : this->capacity() * 2;
    if (new_cap < capacity) new_cap = capacity;

    T* new_ptr = new T[new_cap];
    std::copy(this->data(), this->data() + this->size(), new_ptr);

    if (this->data() != storage_) {
      delete[] this->data();
    }
    this->set(new_ptr, new_cap);
  }

 private:
  void move_from(basic_memory_buffer&& other) {
    if (other.data() == other.storage_) {
      this->set(storage_, SIZE);
      std::copy(other.storage_, other.storage_ + other.size(), storage_);
    } else {
      this->set(other.data(), other.capacity());
      other.set(other.storage_, SIZE);
    }
    // 恢复原有 size
    std::size_t saved_size = other.size();
    other.clear();
    // 修改 this 的 size
    this->append(this->data(), this->data() + saved_size);
  }

  T storage_[SIZE];
};
}  // namespace mini_fmt