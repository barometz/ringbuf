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
#include <type_traits>

namespace baudvine {

// A ring buffer based on std::deque. Inefficient, but compact and easily
// verifiable.
template <typename Elem,
          std::size_t Capacity,
          typename Allocator = std::allocator<Elem>>
class DequeRingBuf {
 public:
  using storage = std::deque<Elem, Allocator>;
  using allocator_type = typename storage::allocator_type;
  using value_type = typename storage::value_type;
  using reference = typename storage::reference;
  using const_reference = typename storage::const_reference;
  using iterator = typename storage::iterator;
  using const_iterator = typename storage::const_iterator;
  using reverse_iterator = typename storage::reverse_iterator;
  using const_reverse_iterator = typename storage::const_reverse_iterator;
  using difference_type = typename storage::difference_type;
  using size_type = typename storage::size_type;

 private:
  storage data_{};

 public:
  DequeRingBuf() = default;
  DequeRingBuf(const allocator_type& alloc) : data_(alloc) {}
  DequeRingBuf(const DequeRingBuf& other, const allocator_type& allocator)
      : data_(other.data_, allocator) {}
  DequeRingBuf(DequeRingBuf&& other, const allocator_type& allocator)
      : data_(std::move(other.data_), allocator) {}

  allocator_type get_allocator() const { return data_.get_allocator(); }

  reference front() { return data_.front(); }
  const_reference front() const { return data_.front(); }
  reference back() { return data_.back(); }
  const_reference back() const { return data_.back(); }
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
  constexpr size_type capacity() const { return Capacity; }

  void clear() noexcept(noexcept(data_.clear())) { return data_.clear(); }
  iterator erase(const_iterator pos) noexcept(noexcept(data_.erase(pos))) {
    return data_.erase(pos);
  }
  iterator erase(const_iterator first,
                 const_iterator last) noexcept(noexcept(data_.erase(first,
                                                                    last))) {
    return data_.erase(first, last);
  }

  void push_front(const_reference value) { emplace_front(value); }
  void push_front(value_type&& value) { emplace_front(std::move(value)); }

  template <typename... Args>
  reference emplace_front(Args&&... args) {
    if (max_size() == 0) {
      return data_.front();
    }

    data_.emplace_front(std::forward<Args>(args)...);

    if (size() > max_size()) {
      pop_back();
    }
    return data_.front();
  }

  void push_back(const_reference value) { emplace_back(value); }
  void push_back(value_type&& value) { emplace_back(std::move(value)); }

  template <typename... Args>
  reference emplace_back(Args&&... args) {
    if (max_size() == 0) {
      return data_.back();
    }

    data_.emplace_back(std::forward<Args>(args)...);

    if (size() > max_size()) {
      pop_front();
    }
    return data_.back();
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
};

}  // namespace baudvine
