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
template <typename Elem>
class DequeRingBuf {
 public:
  using value_type = Elem;
  using reference = Elem&;
  using const_reference = const Elem&;
  using iterator = typename std::deque<value_type>::iterator;
  using const_iterator = typename std::deque<value_type>::const_iterator;
  using difference_type = typename iterator::difference_type;
  using size_type = std::size_t;

  DequeRingBuf() {}
  DequeRingBuf(size_type capacity) : capacity_(capacity) {}

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

  bool empty() const { return data_.empty(); }
  size_type size() const { return data_.size(); }
  size_type max_size() const { return data_.max_size(); }
  size_type capacity() const { return capacity_; }
  void shrink_to_fit() { return data_.shrink_to_fit(); }

  void clear() { return data_.clear(); }

  void push_back(const_reference value) {
    if (capacity() == 0) {
      return;
    }

    if (size() == capacity()) {
      pop_front();
    }

    data_.push_back(value);
  }

  void push_back(value_type&& value) { emplace_back(std::move(value)); }

  template <typename... Args>
  void emplace_back(Args... args) {
    if (capacity() == 0) {
      return;
    }

    if (size() == capacity()) {
      pop_front();
    }

    data_.emplace_back(std::forward<Args>(args)...);
  }

  void pop_front() { data_.pop_front(); }

  void set_capacity(size_type capacity) {
    capacity_ = capacity;
    while (size() > capacity_) {
      pop_front();
    }
  }

  void swap(DequeRingBuf& other) { return std::swap(*this, other); }

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
  size_type capacity_{};
};

}  // namespace baudvine
