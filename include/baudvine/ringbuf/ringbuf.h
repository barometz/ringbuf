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
 *   a variable length. Logical indices are relative to its start
 *   ("ring_offset"), and ring_offset + index may exceed Capacity (before
 *   wrapping).
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <tuple>

namespace baudvine {
namespace detail {

/**
 * @brief Wrap a physical position into an array of size Capacity.
 *
 * Precondition: ring_index < 2 * Capacity.
 *
 * @tparam Capacity The backing array size.
 * @param ring_index The physical index into the backing array.
 * @returns The ring_index wrapped to [0..Capacity).
 */
template <std::size_t Capacity>
constexpr std::size_t RingWrap(const std::size_t ring_index) {
  // This is a bit faster than `return ring_index % Capacity` (~40% reduction in
  // Speed.PushBackOverFull test)
  return (ring_index < Capacity) ? ring_index : ring_index - Capacity;
}

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
   * @param ring_offset Physical index of the start of the ring buffer.
   * @param ring_index Logical index in the ring buffer: when the iterator is at
   *                   ring_offset, ring_index is 0.
   */
  Iterator(pointer data,
           const std::size_t ring_offset,
           const std::size_t ring_index)
      : data_(data), ring_offset_(ring_offset), ring_index_(ring_index) {}

  /**
   * @brief Convert an iterator into a const iterator.
   *
   * @returns A const iterator pointing to the same place.
   */
  explicit operator Iterator<const value_type, Capacity>() const {
    return Iterator<const value_type, Capacity>(data_, ring_offset_,
                                                ring_index_);
  }

  reference operator*() const noexcept {
    return data_[RingWrap<Capacity>(ring_offset_ + ring_index_)];
  }

  pointer operator->() const noexcept { return &**this; }

  Iterator operator++(int) noexcept {
    Iterator copy(*this);
    operator++();
    return copy;
  }

  Iterator& operator++() noexcept {
    ++ring_index_;
    return *this;
  }

  friend bool operator<(const Iterator& lhs, const Iterator& rhs) noexcept {
    // Comparison via std::tie uses std::tuple::operator<, which compares its
    // elements lexicographically.
    return std::tie(lhs.data_, lhs.ring_offset_, lhs.ring_index_) <
           std::tie(rhs.data_, rhs.ring_offset_, rhs.ring_index_);
  }

  friend bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept {
    // Comparison via std::tie is very slow in debug builds, eating into
    // range-for cycle time.
    return lhs.ring_index_ == rhs.ring_index_ && lhs.data_ == rhs.data_ &&
           lhs.ring_offset_ == rhs.ring_offset_;
  }

  friend bool operator!=(const Iterator& lhs, const Iterator& rhs) noexcept {
    return !(lhs == rhs);
  }

  /**
   * @brief Copy the elements in the range [@c begin, @c end) to a destination
   * range starting at @c out.
   *
   * @tparam OutputIt The output iterator type.
   * @param begin Start of the source range.
   * @param end End of the source range, one past the last element to copy.
   * @param out Start of the destination range.
   * @return OutputIt One past the last copied element in the destination range.
   *
   * note: I feel like this should be a friend instead of static, but haven't
   * been able to get it to compile as a friend without moving the definition
   * out of the class definition.
   */
  template <typename OutputIt>
  static OutputIt copy(const Iterator& begin,
                       const Iterator& end,
                       OutputIt out) {
    // TODO: calculate length efficiently
    const auto length = std::distance(begin, end);
    // TODO: assert or no?
    assert(length >= 0);
    const auto range1 = std::min<decltype(length)>(
        length, Capacity - (begin.ring_offset_ + begin.ring_index_));
    const auto beginptr = &*begin;
    out = std::copy(&beginptr[0], &beginptr[range1], out);
    if (range1 == length) {
      return out;
    }
    const auto endptr = &*end;
    out = std::copy(end.data_, endptr, out);
    return out;
  }

 private:
  pointer data_{};
  // Keeping both ring_offset_ and ring_index_ around is algorithmically
  // redundant (you could add them once and then increment the sum in
  // operator++), but the unchanging ring_offset_ appears to help the compiler
  // optimize RingWrap calls.
  std::size_t ring_offset_{};
  std::size_t ring_index_{};
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
  using const_iterator = detail::Iterator<const Elem, Capacity>;
  using difference_type = typename iterator::difference_type;
  using size_type = std::size_t;
  using alloc = std::allocator<value_type>;
  using alloc_traits = std::allocator_traits<alloc>;

  /**
   * @brief Construct a new ring buffer object, and allocate the required
   * memory.
   */
  RingBuf()
      : data_(alloc_traits::allocate(alloc_, Capacity)),
        data_end_(data_ + Capacity){};
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
    std::swap(next_, other.next_);
    std::swap(data_end_, other.data_end_);
    std::swap(ring_offset_, other.ring_offset_);
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
  const_reference operator[](const size_type index) const {
    return data_[detail::RingWrap<Capacity>(ring_offset_ + index)];
  }
  /**
   * @brief Retrieve an element from the ring buffer without range checking.
   *
   * The behaviour is undefined when @c index is outside [0, size()).
   *
   * @param index The logical index into the ring buffer.
   * @return A reference to the element.
   */
  reference operator[](const size_type index) {
    return data_[detail::RingWrap<Capacity>(ring_offset_ + index)];
  }
  /**
   * @brief Retrieve an element from the ring buffer with range checking.
   *
   * @param index The logical index into the ring buffer. Must be in range
   *              [0, size()).
   * @return A const reference to the element.
   * @exception std::out_of_range The index is out of range.
   */
  const_reference at(const size_type index) const {
    if (index >= size()) {
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
  reference at(const size_type index) {
    if (index >= size()) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return (*this)[index];
  }

  /**
   * @return An iterator pointing at the start of the ring buffer.
   */
  iterator begin() noexcept { return iterator(&data_[0], ring_offset_, 0); }
  /**
   * @return An iterator pointing at one past the last element of the ring
   * buffer.
   */
  iterator end() noexcept { return iterator(&data_[0], ring_offset_, size()); }
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
    return const_iterator(&data_[0], ring_offset_, 0);
  }
  /**
   * @return A const iterator pointing at one past the last element of the ring
   * buffer.
   */
  const_iterator cend() const noexcept {
    return const_iterator(&data_[0], ring_offset_, size());
  }

  /**
   * @brief Check if the ring buffer is empty.
   *
   * @return True if size() == 0, otherwise false.
   */
  bool empty() const noexcept { return size() == 0; }
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
   * @brief Push a new element at the front of the ring buffer, popping the back
   * if necessary.
   *
   * @param value The value to copy into the ring buffer.
   */
  void push_front(const_reference value) { return emplace_front(value); }
  /**
   * @brief Push a new element at the front of the ring buffer, popping the
   * front if necessary.
   *
   * @param value The value to move into the ring buffer.
   */
  void push_front(value_type&& value) {
    return emplace_front(std::move(value));
  }
  /**
   * @brief Construct a new element in-place before the front of the ring
   * buffer, popping the back if necessary.
   *
   * @tparam Args Arguments to the element constructor.
   * @param args Arguments to the element constructor.
   */
  template <typename... Args>
  void emplace_front(Args&&... args) {
    if (max_size() == 0) {
      // A buffer of size zero is conceptually sound, so let's support it.
      return;
    }

    // If required, make room first.
    if (size() == max_size()) {
      pop_back();
    }

    GrowFront();
    alloc_traits::construct(alloc_, &data_[ring_offset_],
                            std::forward<Args>(args)...);
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
    if (max_size() == 0) {
      // A buffer of size zero is conceptually sound, so let's support it.
      return;
    }

    // If required, make room first.
    if (size() == max_size()) {
      pop_front();
    }

    alloc_traits::construct(alloc_, &data_[next_], std::forward<Args>(args)...);
    GrowBack();
  }

  /**
   * @brief Pop an element off the front, destroying the first element in the
   * ring buffer.
   */
  void pop_front() {
    if (size() == 0) {
      return;
    }

    alloc_traits::destroy(alloc_, &data_[ring_offset_]);
    ShrinkFront();
  }

  /**
   * @brief Pop an element off the back, destroying the last element in the ring
   * buffer.
   */
  void pop_back() {
    if (size() == 0) {
      return;
    }

    ShrinkBack();
    alloc_traits::destroy(alloc_, &data_[next_]);
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
  // The allocator is used to allocate memory, and to construct and destroy
  // elements.
  alloc alloc_{};

  // The start of the dynamically allocated backing array.
  pointer data_{nullptr};
  // The next position to write to for push_back().
  size_type next_{0U};
  // One past the last element of the dynamically allocated backing array.
  pointer data_end_{nullptr};

  // Start of the ring buffer in data_.
  size_type ring_offset_{0U};
  // The number of elements in the ring buffer (distance between begin() and
  // end()).
  size_type size_{0U};

  constexpr static size_type Decrement(const size_type index) {
    return index > 0 ? index - 1 : Capacity - 1;
  }

  constexpr static size_type Increment(const size_type index) {
    return index < (Capacity - 1) ? index + 1 : 0;
  }

  // Move things after pop_front.
  void ShrinkFront() {
    ring_offset_ = Increment(ring_offset_);
    // Precondition: size != 0 (when it is, pop_front returns early.)
    size_--;
  }

  // Move things around before pop_back destroys the last entry.
  void ShrinkBack() {
    next_ = Decrement(next_);
    // Precondition: size != 0 (when it is, pop_back returns early.)
    size_--;
  }

  // Move things around before emplace_front constructs its new entry.
  void GrowFront() {
    // Move ring_offset_ down, and possibly around
    ring_offset_ = Decrement(ring_offset_);
    // Precondition: size != Capacity (when it is, emplace_front pop_backs
    // first.)
    size_++;
  }

  // Move things around after emplace_back.
  void GrowBack() {
    next_ = Increment(next_);
    // Precondition: size != Capacity (when it is, emplace_back pop_fronts
    // first)
    size_++;
  }
};

template <typename Elem, std::size_t Capacity, typename OutputIt>
OutputIt copy(const detail::Iterator<Elem, Capacity>& begin,
              const detail::Iterator<Elem, Capacity>& end,
              OutputIt out) {
  return detail::Iterator<Elem, Capacity>::copy(begin, end, out);
}

}  // namespace baudvine
