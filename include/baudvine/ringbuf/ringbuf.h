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

/**
 * @file ringbuf.h
 *
 * A ring buffer for C++11, with an STL-compatible interface.
 *
 * The comments frequently refer to "physical" and "logical" indices. This is
 * meant to make explicit the distinction between:
 *
 * - The backing array of RingBuf, which is always of size Capacity and is
 *   allocated once during RingBuf construction. Physical indices are relative
 *   to the start of this array.
 * - The conceptual ring buffer, which moves around in the backing array and has
 *   a variable length. Logical indices are relative to its start ("base"), and
 *   base + index may exceed Capacity (before wrapping).
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <tuple>

namespace baudvine {
namespace detail {

/**
 * @brief Wrap a physical position into an array of size Capacity.
 *
 * Precondition: position < 2 * Capacity.
 *
 * @tparam Capacity The backing array size.
 * @param position The physical index into the backing array.
 * @returns The position wrapped to [0..Capacity).
 */
template <std::size_t Capacity>
constexpr std::size_t RingWrap(const std::size_t position) {
  // This is a bit faster than `return position % Capacity` (~40% reduction in
  // Speed.PushBackOverFull test)
  return (position < Capacity) ? position : position - Capacity;
}

/**
 * @brief A const iterator into RingBuf. A const iterator can't be used to
 * modify the value it points to.
 *
 * @tparam Elem The element type this iterator points to.
 * @tparam Capacity The size of the backing array, and maximum size of the ring
 *         buffer.
 */
template <typename Elem, std::size_t Capacity>
class ConstIterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Elem;
  using pointer = const Elem*;
  using reference = const Elem&;
  using iterator_category = std::forward_iterator_tag;

  constexpr ConstIterator() noexcept = default;
  /**
   * @brief Construct a new const iterator object.
   *
   * @param data Pointer to the start of the RingBuf's backing array.
   * @param base Physical index of the start of the ring buffer.
   * @param position Logical index in the ring buffer: when the iterator is at
   *                 base, position is 0.
   */
  ConstIterator(pointer data,
                const std::size_t base,
                const std::size_t position) noexcept
      : data_(data), base_(base), position_(position) {}

  reference operator*() const noexcept {
    return data_[RingWrap<Capacity>(base_ + position_)];
  }

  pointer operator->() const noexcept { return &**this; }

  ConstIterator operator++(int) noexcept {
    ConstIterator copy(*this);
    operator++();
    return copy;
  }

  ConstIterator& operator++() noexcept {
    ++position_;
    return *this;
  }

  friend bool operator<(const ConstIterator& lhs,
                        const ConstIterator& rhs) noexcept {
    // Comparison via std::tie uses std::tuple::operator<, which compares its
    // elements lexicographically.
    return std::tie(lhs.data_, lhs.base_, lhs.position_) <
           std::tie(rhs.data_, rhs.base_, rhs.position_);
  }

  friend bool operator==(const ConstIterator& lhs,
                         const ConstIterator& rhs) noexcept {
    // Comparison via std::tie is very slow in debug builds, eating into
    // range-for cycle time.
    return lhs.position_ == rhs.position_ && lhs.data_ == rhs.data_ &&
           lhs.base_ == rhs.base_;
  }

  friend bool operator!=(const ConstIterator& lhs,
                         const ConstIterator& rhs) noexcept {
    return !(lhs == rhs);
  }

 private:
  pointer data_{};
  // Keeping both base_ and position_ around is algorithmically redundant (you
  // could add them once and then increment the sum in operator++), but the
  // unchanging base_ appears to help the compiler optimize RingWrap calls.
  std::size_t base_{};
  std::size_t position_{};
};

/**
 * @brief An iterator into RingBuf.
 *
 * @tparam Elem The element type this iterator points to.
 * @tparam Capacity The size of the backing array, and maximum size of the ring
 *         buffer.
 */
template <typename Elem, size_t Capacity>
class Iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Elem;
  using pointer = Elem*;
  using reference = Elem&;
  using iterator_category = std::forward_iterator_tag;

  constexpr Iterator() noexcept = default;
  /**
   * @brief Construct a new iterator object.
   *
   * @param data Pointer to the start of the RingBuf's backing array.
   * @param base Physical index of the start of the ring buffer.
   * @param position Logical index in the ring buffer: when the iterator is at
   *                 base, position is 0.
   */
  Iterator(pointer data, const std::size_t base, const size_t position)
      : data_(data), base_(base), position_(position) {}

  /**
   * @brief Convert an iterator into a const iterator.
   *
   * @returns A const iterator pointing to the same place.
   */
  explicit operator ConstIterator<value_type, Capacity>() const {
    return ConstIterator<value_type, Capacity>(data_, base_, position_);
  }

  reference operator*() const noexcept {
    return data_[RingWrap<Capacity>(base_ + position_)];
  }

  pointer operator->() const noexcept { return &**this; }

  Iterator operator++(int) noexcept {
    Iterator copy(*this);
    operator++();
    return copy;
  }

  Iterator& operator++() noexcept {
    ++position_;
    return *this;
  }

  friend bool operator<(const Iterator& lhs, const Iterator& rhs) noexcept {
    // Comparison via std::tie uses std::tuple::operator<, which compares its
    // elements lexicographically.
    return std::tie(lhs.data_, lhs.base_, lhs.position_) <
           std::tie(rhs.data_, rhs.base_, rhs.position_);
  }

  friend bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept {
    // Comparison via std::tie is very slow in debug builds, eating into
    // range-for cycle time.
    return lhs.position_ == rhs.position_ && lhs.data_ == rhs.data_ &&
           lhs.base_ == rhs.base_;
  }

  friend bool operator!=(const Iterator& lhs, const Iterator& rhs) noexcept {
    return !(lhs == rhs);
  }

 private:
  pointer data_{};
  // Keeping both base_ and position_ around is algorithmically redundant (you
  // could add them once and then increment the sum in operator++), but the
  // unchanging base_ appears to help the compiler optimize RingWrap calls.
  std::size_t base_{};
  std::size_t position_{};
};

}  // namespace detail

/**
 * @brief The ring buffer itself.
 *
 * @tparam Elem The type of elements contained by the ring buffer.
 * @tparam Capacity The maximum size of the ring buffer, and the fixed size of
 *         the backing array.
 */
template <typename Elem, size_t Capacity>
class RingBuf {
 public:
  using value_type = Elem;
  using reference = Elem&;
  using pointer = Elem*;
  using const_reference = const Elem&;
  using iterator = detail::Iterator<Elem, Capacity>;
  using const_iterator = detail::ConstIterator<Elem, Capacity>;
  using difference_type = typename iterator::difference_type;
  using size_type = std::size_t;
  using alloc = std::allocator<value_type>;
  using alloc_traits = std::allocator_traits<alloc>;

  /**
   * @brief Construct a new ring buffer object, and allocate the required
   * memory.
   */
  RingBuf() : data_(alloc_traits::allocate(alloc_, Capacity)){};
  /**
   * @brief Destroy the ring buffer object.
   *
   * Destroys the active elements via clear() and deallocates the backing array.
   */
  ~RingBuf() {
    clear();
    alloc_traits::deallocate(alloc_, data_, Capacity);
  }
  /**
   * @brief Construct a new RingBuf object out of another, using elementwise
   * copy assignment.
   *
   * @param other The RingBuf to copy values from.
   * @todo maybe allow other (smaller) sizes as input?
   */
  RingBuf(const RingBuf& other) : RingBuf() { *this = other; }
  /**
   * @brief Construct a new RingBuf object out of another, using bulk move
   * assignment.
   *
   * @param other The RingBuf to move the data out of.
   */
  RingBuf(RingBuf&& other) noexcept { *this = std::move(other); }

  /**
   * @brief Copy a RingBuf into this one.
   *
   * First clears this RingBuf, and then copies @c other element by element.
   *
   * @param other The RingBuf to copy from.
   * @return This RingBuf.
   */
  RingBuf& operator=(const RingBuf& other) {
    clear();
    for (const auto& value : other) {
      push_back(value);
    }
    return *this;
  }
  /**
   * @brief Move a RingBuf into this one.
   *
   * The backing storage is swapped, so no elementwise moves are performed.
   *
   * @param other The RingBuf to copy from.
   * @return This RingBuf.
   */
  RingBuf& operator=(RingBuf&& other) noexcept {
    std::swap(alloc_, other.alloc_);
    std::swap(data_, other.data_);
    std::swap(base_, other.base_);
    std::swap(size_, other.size_);
    return *this;
  }

  /**
   * @brief Return the first element in the ring buffer.
   *
   * @return The first element in the ring buffer.
   * @exception std::out_of_range The buffer is empty.
   */
  reference front() { return at(0); }
  /**
   * @brief Return the last element in the ring buffer.
   *
   * @return The last element in the ring buffer.
   * @exception std::out_of_range The buffer is empty.
   */
  reference back() { return at(size() - 1); }

  /**
   * @brief Retrieve an element from the ring buffer without range checking.
   *
   * The behaviour is undefined when @c index is outside [0, size()).
   *
   * @param index The logical index into the ring buffer.
   * @return A const reference to the element.
   */
  const_reference operator[](size_type index) const {
    return data_[detail::RingWrap<Capacity>(base_ + index)];
  }
  /**
   * @brief Retrieve an element from the ring buffer without range checking.
   *
   * The behaviour is undefined when @c index is outside [0, size()).
   *
   * @param index The logical index into the ring buffer.
   * @return A reference to the element.
   */
  reference operator[](size_type index) {
    return data_[detail::RingWrap<Capacity>(base_ + index)];
  }
  /**
   * @brief Retrieve an element from the ring buffer with range checking.
   *
   * @param index The logical index into the ring buffer. Must be in range
   *              [0, size()).
   * @return A const reference to the element.
   * @exception std::out_of_range The index is out of range.
   */
  const_reference at(size_type index) const {
    if (index >= this->size()) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return (*this)[index];
  }
  /**
   * @brief Retrieve an element from the ring buffer with range checking.
   *
   * @param index The logical index into the ring buffer. Must be in range
   *              [0, size()).
   * @return A reference to the element.
   * @exception std::out_of_range The index is out of range.
   */
  reference at(size_type index) {
    if (index >= this->size()) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return (*this)[index];
  }

  /**
   * @return An iterator pointing at the start of the ring buffer.
   */
  iterator begin() noexcept {
    return iterator(&this->data_[0], this->base_, 0);
  }
  /**
   * @return An iterator pointing at one past the last element of the ring
   * buffer.
   */
  iterator end() noexcept {
    return iterator(&this->data_[0], this->base_, this->size());
  }
  /**
   * @return A const iterator pointing at the start of the ring buffer.
   */
  const_iterator begin() const noexcept { return cbegin(); }
  /**
   * @return A const iterator pointing at one past the last element of the ring
   * buffer.
   */
  const_iterator end() const noexcept { return cend(); }
  /**
   * @return A const iterator pointing at the start of the ring buffer.
   */
  const_iterator cbegin() const noexcept {
    return const_iterator(&this->data_[0], this->base_, 0);
  }
  /**
   * @return A const iterator pointing at one past the last element of the ring
   * buffer.
   */
  const_iterator cend() const noexcept {
    return const_iterator(&this->data_[0], this->base_, this->size());
  }

  /**
   * @brief Check if the ring buffer is empty.
   *
   * @return True if size() == 0, otherwise false.
   */
  bool empty() const noexcept { return this->size() == 0; }
  /**
   * @brief Get the current number of elements in the ring buffer.
   *
   * @return The number of elements.
   */
  size_type size() const noexcept { return size_; }
  /**
   * @brief Get the maximum number of elements in this ring buffer.
   *
   * @return Capacity.
   */
  constexpr size_type max_size() const noexcept { return Capacity; }
  /**
   * @brief
   *
   * @return constexpr size_type
   * @todo remove this, max_size is enough
   */
  constexpr size_type capacity() const noexcept { return Capacity; }

  /**
   * @brief Remove all elements from the ring buffer, destroying each one
   * starting at the front.
   *
   * After clear(), size() == 0.
   */
  void clear() {
    // It might be fractionally more efficient to iterate through begin..end and
    // allocator::destroy each one, but this is a lot nicer to read.
    while (!empty()) {
      pop_front();
    }
  }

  /**
   * @brief Push a new element into the ring buffer, popping the front if
   * necessary.
   *
   * @param value The value to copy into the ring buffer.
   */
  void push_back(const_reference value) { return emplace_back(value); }
  /**
   * @brief Push a new element into the ring buffer, popping the front if
   * necessary.
   *
   * @param value The value to move into the ring buffer.
   */
  void push_back(value_type&& value) { return emplace_back(std::move(value)); }
  /**
   * @brief Construct a new element in-place at the end of the ring buffer,
   * popping the front if necessary.
   *
   * @tparam Args Arguments to the element constructor.
   * @param args Arguments to the element constructor.
   */
  template <typename... Args>
  void emplace_back(Args&&... args) {
    if (Capacity == 0) {
      // A buffer of size zero is conceptually sound, so let's support it.
      return;
    }

    if (size_ == Capacity) {
      pop_front();
    }

    alloc_traits::construct(alloc_, &data_[GetNext()],
                            std::forward<Args>(args)...);
    Progress();
  }

  /**
   * @brief Pop the front, destroying the first element in the ring buffer.
   */
  void pop_front() {
    if (size_ == 0) {
      return;
    }

    alloc_traits::destroy(alloc_, &data_[base_]);

    base_ = detail::RingWrap<Capacity>(base_ + 1);
    size_--;
  }

  /**
   * @brief Swap this ring buffer with another using std::swap.
   *
   * @param other The RingBuf to swap with.
   */
  void swap(RingBuf& other) noexcept { std::swap(*this, other); }

  /**
   * @brief Elementwise lexicographical comparison of two ring buffers.
   *
   * @param lhs The left-hand side in lhs < rhs.
   * @param rhs The right-hand side in lhs < rhs.
   * @returns True if the left-hand side compares as less than the right.
   */
  friend bool operator<(const RingBuf& lhs, const RingBuf& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
  }
  /**
   * @brief Elementwise lexicographical comparison of two ring buffers.
   *
   * @param lhs The left-hand side in lhs > rhs.
   * @param rhs The right-hand side in lhs > rhs.
   * @returns True if the left-hand side compares as more than the right.
   */
  friend bool operator>(const RingBuf& lhs, const RingBuf& rhs) {
    return rhs < lhs;
  }
  /**
   * @brief Elementwise comparison of two ring buffers.
   *
   * @param lhs The left-hand side in lhs == rhs.
   * @param rhs The right-hand side in lhs == rhs.
   * @returns True if @c lhs is equal to @c rhs.
   */
  friend bool operator==(const RingBuf& lhs, const RingBuf& rhs) {
    if (lhs.size() != rhs.size()) {
      return false;
    }

    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }
  /**
   * @brief Elementwise comparison of two ring buffers.
   *
   * @param lhs The left-hand side in lhs >= rhs.
   * @param rhs The right-hand side in lhs >= rhs.
   * @returns True if @c lhs is greater than or equal to @c rhs.
   */
  friend bool operator>=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs < rhs);
  }
  /**
   * @brief Elementwise comparison of two ring buffers.
   *
   * @param lhs The left-hand side in lhs <= rhs.
   * @param rhs The right-hand side in lhs <= rhs.
   * @returns True if @c lhs is less than or equal to @c rhs.
   */
  friend bool operator<=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs > rhs);
  }
  /**
   * @brief Elementwise comparison of two ring buffers.
   *
   * @param lhs The left-hand side in lhs != rhs.
   * @param rhs The right-hand side in lhs != rhs.
   * @returns True if @c lhs is not equal to @c rhs.
   */
  friend bool operator!=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs == rhs);
  }

 private:
  alloc alloc_{};
  pointer data_{nullptr};
  size_type base_{0U};
  size_type size_{0U};

  size_type GetNext() { return detail::RingWrap<Capacity>(base_ + size_); }

  // Move things around after growing.
  void Progress() {
    // The base only moves when we're full.
    if (size_ == Capacity) {
      base_ = detail::RingWrap<Capacity>(base_ + 1);
    }

    // Size will never exceed the capacity.
    if (size_ < Capacity) {
      size_++;
    }
  }
};
}  // namespace baudvine
