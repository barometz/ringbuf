// Copyright © 2021 Dominic van Berkel <dominic@baudvine.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstddef>
#include <deque>

namespace baudvine {

// A ring buffer based on std::deque. Inefficient, but compact and easily
// verifiable.
template <typename Elem, std::size_t Capacity>
class DequeRingBuf {
 public:
  using value_type = Elem;
  using reference = Elem&;
  using const_reference = const Elem&;
  using iterator = typename std::deque<value_type>::iterator;
  using const_iterator = typename std::deque<value_type>::const_iterator;
  using reverse_iterator = typename std::deque<value_type>::reverse_iterator;
  using const_reverse_iterator =
      typename std::deque<value_type>::const_reverse_iterator;
  using difference_type = typename iterator::difference_type;
  using size_type = std::size_t;

  DequeRingBuf() = default;

  reference front() { return data_.front(); }
  reference back() { return data_.back(); }
  reference at(size_type index) { return data_.at(index); }
  const_reference at(size_type index) const { return data_.at(index); }
  reference operator[](size_type index) { return data_[index]; }
  const_reference operator[](size_type index) const { return data_[index]; }

  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }
  const_iterator begin() const { return data_.begin(); }
  const_iterator end() const { return data_.end(); }
  const_iterator cbegin() const { return data_.cbegin(); }
  const_iterator cend() const { return data_.cend(); }
  reverse_iterator rbegin() { return data_.rbegin(); }
  reverse_iterator rend() { return data_.rend(); }
  const_reverse_iterator rbegin() const { return data_.rbegin(); }
  const_reverse_iterator rend() const { return data_.rend(); }
  const_reverse_iterator crbegin() const { return data_.crbegin(); }
  const_reverse_iterator crend() const { return data_.crend(); }

  bool empty() const { return data_.empty(); }
  size_type size() const { return data_.size(); }
  constexpr size_type max_size() const { return Capacity; }

  void clear() noexcept(noexcept(data_.clear())) { return data_.clear(); }

  void push_front(const_reference value) { return emplace_front(value); }
  void push_front(value_type&& value) {
    return emplace_front(std::move(value));
  }

  template <typename... Args>
  void emplace_front(Args... args) {
    if (max_size() == 0) {
      return;
    }

    if (size() == max_size()) {
      pop_back();
    }

    data_.emplace_front(std::forward<Args>(args)...);
  }

  void push_back(const_reference value) { return emplace_back(value); }
  void push_back(value_type&& value) { return emplace_back(std::move(value)); }

  template <typename... Args>
  void emplace_back(Args... args) {
    if (max_size() == 0) {
      return;
    }

    if (size() == max_size()) {
      pop_front();
    }

    data_.emplace_back(std::forward<Args>(args)...);
  }

  void pop_front() noexcept(noexcept(data_.pop_front())) {
    return data_.pop_front();
  }
  void pop_back() noexcept(noexcept(data_.pop_back())) {
    return data_.pop_back();
  }

  void swap(DequeRingBuf& other) noexcept(noexcept(data_.swap(other.data_))) {
    return data_.swap(other.data_);
  }

  friend bool operator<(const DequeRingBuf& lhs, const DequeRingBuf& rhs) {
    return lhs.data_ < rhs.data_;
  }
  friend bool operator>(const DequeRingBuf& lhs, const DequeRingBuf& rhs) {
    return lhs.data_ > rhs.data_;
  }
  friend bool operator<=(const DequeRingBuf& lhs, const DequeRingBuf& rhs) {
    return lhs.data_ <= rhs.data_;
  }
  friend bool operator>=(const DequeRingBuf& lhs, const DequeRingBuf& rhs) {
    return lhs.data_ >= rhs.data_;
  }
  friend bool operator==(const DequeRingBuf& lhs, const DequeRingBuf& rhs) {
    return lhs.data_ == rhs.data_;
  }
  friend bool operator!=(const DequeRingBuf& lhs, const DequeRingBuf& rhs) {
    return lhs.data_ != rhs.data_;
  }

 private:
  std::deque<value_type> data_{};
};

}  // namespace baudvine
