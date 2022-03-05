// Copyright © 2021 Dominic van Berkel <dominic@baudvine.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace baudvine {
namespace detail {
template <typename Elem, typename Allocator, typename RingBuf>
class BaseRingBuf {
 protected:
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<allocator_type>;
  using value_type = Elem;
  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;
  using reference = decltype(*pointer{});
  using const_reference = decltype(*const_pointer{});
  using difference_type = typename alloc_traits::difference_type;
  using size_type = typename alloc_traits::size_type;
  using unsigned_difference =
      typename std::make_unsigned<difference_type>::type;

  RingBuf* Impl() noexcept { return static_cast<RingBuf*>(this); }
  const RingBuf* Impl() const noexcept {
    return static_cast<const RingBuf*>(this);
  }

  reference UncheckedAt(size_type index) { return (*Impl())[index]; }
  const_reference UncheckedAt(size_type index) const {
    return (*Impl())[index];
  }

  /**
   * Retrieve an element from the ring buffer with range checking.
   *
   * @param index The logical index into the ring buffer. Must be in range
   *              [0, size()).
   * @returns A const reference to the element.
   * @throws std::out_of_range The index is out of range.
   */
  const_reference at(const size_type index) const {
    if (index >= Impl()->size()) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return UncheckedAt(index);
  }

  /**
   * Retrieve an element from the ring buffer with range checking.
   *
   * @param index The logical index into the ring buffer. Must be in range
   *              [0, size()).
   * @returns A reference to the element.
   * @throws std::out_of_range The index is out of range.
   */
  reference at(const size_type index) {
    if (index >= Impl()->size()) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return UncheckedAt(index);
  }

  /**
   * Returns the first element in the ring buffer.
   * @throws std::out_of_range The buffer is empty.
   */
  reference front() { return at(0); }
  /**
   * Returns the first element in the ring buffer.
   * @throws std::out_of_range The buffer is empty.
   */
  const_reference front() const { return at(0); }
  /**
   * Returns he last element in the ring buffer.
   * @throws std::out_of_range The buffer is empty.
   */
  reference back() { return at(Impl()->size() - 1); }
  /**
   * Returns he last element in the ring buffer.
   * @throws std::out_of_range The buffer is empty.
   */
  const_reference back() const { return at(Impl()->size() - 1); }

  /**
   * Get whether the ring buffer is empty (size() == 0)
   */
  bool empty() const noexcept { return Impl()->size() == 0; }

  /**
   * Push a new element at the front of the ring buffer, popping the back if
   * necessary.
   *
   * @param value The value to copy into the ring buffer.
   */
  void push_front(const_reference value) { Impl()->emplace_front(value); }
  /**
   * Push a new element at the front of the ring buffer, popping the back if
   * necessary.
   *
   * @param value The value to move into the ring buffer.
   */
  void push_front(value_type&& value) {
    Impl()->emplace_front(std::move(value));
  }

  /**
   * Push a new element into the ring buffer, popping the front if necessary.
   *
   * @param value The value to copy into the ring buffer.
   */
  void push_back(const_reference value) { Impl()->emplace_back(value); }
  /**
   * Push a new element into the ring buffer, popping the front if necessary.
   *
   * @param value The value to move into the ring buffer.
   */
  void push_back(value_type&& value) { Impl()->emplace_back(std::move(value)); }

  /**
   * Remove all elements from the ring buffer, destroying each one starting at
   * the front.
   *
   * After clear(), size() == 0.
   */
  void clear() noexcept(noexcept(Impl()->pop_front())) {
    // It might be fractionally more efficient to iterate through begin..end and
    // allocator::destroy each one, but this is a lot nicer to read.
    while (!Impl()->empty()) {
      Impl()->pop_front();
    }
  }

  /**
   * Elementwise lexicographical comparison of two ring buffers.
   *
   * @returns True if the left-hand side compares as less than the right.
   */
  friend bool operator<(const RingBuf& lhs, const RingBuf& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
  }
  /**
   * Elementwise lexicographical comparison of two ring buffers.
   *
   * @returns True if the left-hand side compares as more than the right.
   */
  friend bool operator>(const RingBuf& lhs, const RingBuf& rhs) {
    return rhs < lhs;
  }
  /**
   * Elementwise comparison of two ring buffers.
   *
   * @returns True if @c lhs is equal to @c rhs.
   */
  friend bool operator==(const RingBuf& lhs, const RingBuf& rhs) {
    if (lhs.size() != rhs.size()) {
      return false;
    }

    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }
  /**
   * Elementwise comparison of two ring buffers.
   *
   * @returns True if @c lhs is greater than or equal to @c rhs.
   */
  friend bool operator>=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs < rhs);
  }
  /**
   * Elementwise comparison of two ring buffers.
   *
   * @returns True if @c lhs is less than or equal to @c rhs.
   */
  friend bool operator<=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs > rhs);
  }
  /**
   * Elementwise comparison of two ring buffers.
   *
   * @returns True if @c lhs is not equal to @c rhs.
   */
  friend bool operator!=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs == rhs);
  }
};
}  // namespace detail
}  // namespace baudvine