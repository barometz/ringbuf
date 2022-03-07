// Copyright © 2022 Dominic van Berkel <dominic@baudvine.net>
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
 * @file flexible_ringbuf.h
 * @copyright MIT License
 *
 * A ring buffer for C++11, with an STL-like interface.
 *
 * The comments frequently refer to "physical" and "logical" indices. This is
 * meant to make explicit the distinction between:
 *
 * - The backing array of FlexRingBuf, which is always of size capacity_ + 1 and
 * is allocated once during FlexRingBuf construction. Physical indices are
 * relative to the start of this array.
 * - The conceptual ring buffer, which moves around in the backing array and has
 *   a variable length. Logical indices are relative to its start
 *   ("ring_offset"), and ring_offset + index may exceed capacity_ (before
 *   wrapping).
 */

#pragma once

#include "base_ringbuf.h"

#include <cstddef>
#include <iterator>
#include <limits>
#include <tuple>

/** The baudvine "project". */
namespace baudvine {
namespace detail {
namespace flexible_ringbuf {
/**
 * Wrap a physical position into an array of size capacity_.
 *
 * Precondition: ring_index < 2 * capacity_ + 1.
 *
 * @param capacity The backing array size.
 * @param ring_index The physical index into the backing array.
 * @returns The ring_index wrapped to [0..capacity_].
 * @private
 */
constexpr std::size_t RingWrap(const std::size_t capacity,
                               const std::size_t ring_index) {
  // This is a bit faster than `return ring_index % capacity_` (~40% reduction
  // in Speed.PushBackOverFull test)
  return (ring_index <= capacity) ? ring_index : ring_index - capacity - 1;
}

/**
 * An iterator into FlexRingBuf.
 *
 * @tparam Ptr The pointer type, which determines constness.
 * @tparam AllocTraits The allocator traits for the container, used for
 *                     size/difference_type and const_pointer (for auto
 *                     conversion to const iterator).
 * @tparam capacity_ The size of the backing array, and maximum size of the ring
 *                  buffer.
 */
template <typename Ptr, typename AllocTraits>
class Iterator : public ringbuf::BaseIterator<Ptr,
                                              AllocTraits,
                                              Iterator<Ptr, AllocTraits>> {
 public:
  using Base = ringbuf::BaseIterator<Ptr, AllocTraits, Iterator>;
  using typename Base::difference_type;
  using typename Base::pointer;
  using typename Base::reference;
  using typename Base::size_type;
  using typename Base::value_type;
  using iterator_category = std::random_access_iterator_tag;

  constexpr Iterator() noexcept = default;
  /**
   * Construct a new iterator object.
   *
   * @param data Pointer to the start of the FlexRingBuf's backing array.
   * @param ring_offset Physical index of the start of the ring buffer.
   * @param ring_index Logical index in the ring buffer: when the iterator is at
   *                   ring_offset, ring_index is 0.
   */
  Iterator(pointer data,
           const size_type capacity,
           const size_type ring_offset,
           const size_type ring_index)
      : data_(data),
        capacity_{capacity},
        ring_offset_(ring_offset),
        ring_index_(ring_index) {}

  /**
   * Convert an iterator into a const iterator.
   *
   * @returns A const iterator pointing to the same place.
   */
  operator Iterator<typename AllocTraits::const_pointer, AllocTraits>() const {
    return Iterator<typename AllocTraits::const_pointer, AllocTraits>(
        data_, capacity_, ring_offset_, ring_index_);
  }

  reference operator*() const {
    BAUDVINE_RINGBUF_ASSERT(ring_index_ <= capacity_);
    return data_[RingWrap(capacity_, ring_offset_ + ring_index_)];
  }

  using Base::operator->;
  using Base::operator[];
  using Base::operator++;
  using Base::operator--;
  using Base::operator-;
  using Base::operator+;

  Iterator& operator+=(difference_type n) noexcept {
    ring_index_ += n;
    return *this;
  }

  Iterator& operator-=(difference_type n) noexcept {
    ring_index_ -= n;
    return *this;
  }

  friend difference_type operator-(const Iterator& lhs,
                                   const Iterator& rhs) noexcept {
    difference_type difference{};
    if (lhs.ring_index_ > rhs.ring_index_) {
      const difference_type distance = lhs.ring_index_ - rhs.ring_index_;
      difference = distance;
    } else {
      const difference_type distance = rhs.ring_index_ - lhs.ring_index_;
      difference = -distance;
    }
    return difference;
  }

  friend bool operator<(const Iterator& lhs, const Iterator& rhs) noexcept {
    return lhs.ring_index_ < rhs.ring_index_;
  }

  template <typename P, typename A, typename OutputIt>
  // https://github.com/llvm/llvm-project/issues/47430
  // NOLINTNEXTLINE(readability-redundant-declaration)
  friend OutputIt copy(const Iterator<P, A>& begin,
                       const Iterator<P, A>& end,
                       OutputIt out);

 private:
  pointer data_{};

  // Keeping both ring_offset_ and ring_index_ around is a little redundant,
  // algorithmically, but it makes it much easier to express iterator-mutating
  // operations.

  // Maximum number of elements in the container.
  size_type capacity_{};
  // Physical index of begin().
  size_type ring_offset_{};
  // Logical index of this iterator.
  size_type ring_index_{};
};

/**
 * @see baudvine::copy
 * @private
 */
template <typename Ptr, typename AllocTraits, typename OutputIt>
OutputIt copy(const Iterator<Ptr, AllocTraits>& begin,
              const Iterator<Ptr, AllocTraits>& end,
              OutputIt out) {
  if (begin == end) {
    // Empty range, pass
  } else if (&end[0] > &begin[0]) {
    // Fully contiguous range.
    out = std::copy(&begin[0], &end[0], out);
  } else {
    // Copy in two sections.
    out = std::copy(&begin[0], &begin.data_[begin.capacity_ + 1], out);
    out = std::copy(end.data_, &end[0], out);
  }

  return out;
}
}  // namespace flexible_ringbuf
}  // namespace detail

/**
 * An STL-like ring buffer with dynamic allocation and compile-time capacity
 * limits.
 *
 * @tparam Elem The type of elements contained by the ring buffer.
 * @tparam capacity_ The maximum size of the ring buffer, and the fixed size of
 *         the backing array.
 * @tparam Allocator The allocator type to use for storage and element
           construction.
 */
template <typename Elem, typename Allocator = std::allocator<Elem>>
class FlexRingBuf
    : public detail::ringbuf::
          BaseRingBuf<Elem, Allocator, FlexRingBuf<Elem, Allocator>> {
 public:
  using Base = detail::ringbuf::BaseRingBuf<Elem, Allocator, FlexRingBuf>;

  using typename Base::alloc_traits;
  using typename Base::allocator_type;
  using typename Base::const_pointer;
  using typename Base::const_reference;
  using typename Base::pointer;
  using typename Base::reference;
  using typename Base::value_type;
  using iterator = detail::flexible_ringbuf::Iterator<pointer, alloc_traits>;
  using const_iterator =
      detail::flexible_ringbuf::Iterator<const_pointer, alloc_traits>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using typename Base::difference_type;
  using typename Base::size_type;
  using typename Base::unsigned_difference;

 private:
  friend Base;

  // The allocator is used to allocate memory, and to construct and destroy
  // elements.
  allocator_type alloc_{};

  // The number of elements that may be stored.
  size_type capacity_{0U};
  // The start of the dynamically allocated backing array.
  pointer data_{nullptr};
  // The next position to write to for push_back().
  size_type next_{0U};

  // Start of the ring buffer in data_.
  size_type ring_offset_{0U};
  // The number of elements in the ring buffer (distance between begin() and
  // end()).
  size_type size_{0U};

  size_type Decrement(const size_type index) const {
    return index > 0 ? index - 1 : capacity_;
  }

  size_type Increment(const size_type index) const {
    return index < (capacity_) ? index + 1 : 0;
  }

  // Swap everything but the allocator - caller has to figure that out
  // separately.
  void Swap(FlexRingBuf& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(capacity_, other.capacity_);
    std::swap(next_, other.next_);
    std::swap(ring_offset_, other.ring_offset_);
    std::swap(size_, other.size_);
  }

  // Move things after pop_front.
  void ShrinkFront() noexcept {
    ring_offset_ = Increment(ring_offset_);
    // Precondition: size != 0 (when it is, pop_front returns early.)
    size_--;
  }

  // Move things around before pop_back destroys the last entry.
  void ShrinkBack() noexcept {
    next_ = Decrement(next_);
    // Precondition: size != 0 (when it is, pop_back returns early.)
    size_--;
  }

  // Move things around before emplace_front constructs its new entry.
  void GrowFront() noexcept {
    // Move ring_offset_ down, and possibly around
    ring_offset_ = Decrement(ring_offset_);
    // Precondition: size != capacity_ (when it is, emplace_front pop_backs
    // first.)
    size_++;
  }

  // Move things around after emplace_back.
  void GrowBack() noexcept {
    next_ = Increment(next_);
    // Precondition: size != capacity_ (when it is, emplace_back pop_fronts
    // first)
    size_++;
  }

  // Construct an element before front().
  template <typename... Args>
  void ConstructFront(Args&&... args) {
    alloc_traits::construct(alloc_, &data_[Decrement(ring_offset_)],
                            std::forward<Args>(args)...);
  }

  // Construct an element after back().
  template <typename... Args>
  void ConstructBack(Args&&... args) {
    alloc_traits::construct(alloc_, &data_[next_], std::forward<Args>(args)...);
  }

  iterator UnConstIterator(const_iterator it) const {
    return iterator(data_, capacity_, ring_offset_, it - begin());
  }

 public:
  /**
   * Construct a new ring buffer object with a default-constructed allocator,
   * and no capacity.
   */
  FlexRingBuf() : FlexRingBuf(allocator_type{}){};

  /**
   * Construct a new ring buffer object with a default-constructed allocator,
   * and allocate the required memory.
   *
   * Allocates capacity_ + 1 to allow for strong exception guarantees in
   * emplace_front/back.
   */
  explicit FlexRingBuf(size_type capacity)
      : FlexRingBuf(capacity, allocator_type{}){};

  /**
   * Construct a new ring buffer object with the provided allocator, and
   * no capacity.
   *
   * @param allocator The allocator to use for storage and element construction.
   */
  explicit FlexRingBuf(const allocator_type& allocator)
      : FlexRingBuf(0, allocator) {}

  /**
   * Construct a new ring buffer object with the provided allocator, and
   * allocate the required memory.
   *
   * Allocates capacity_ + 1 to allow for strong exception guarantees in
   * emplace_front/back.
   *
   * @param allocator The allocator to use for storage and element construction.
   */
  FlexRingBuf(size_type capacity, const allocator_type& allocator)
      : alloc_(allocator),
        capacity_(capacity),
        data_(alloc_traits::allocate(alloc_, capacity_ + 1)) {}

  /**
   * Destroy the ring buffer object.
   *
   * Destroys the active elements via clear() and deallocates the backing array.
   */
  ~FlexRingBuf() {
    clear();
    alloc_traits::deallocate(alloc_, data_, capacity_ + 1);
  }
  /**
   * Construct a new FlexRingBuf object out of another, using elementwise
   * copy assignment.
   *
   * @param other The FlexRingBuf to copy values from.
   * @todo maybe allow other (smaller) sizes as input?
   */
  FlexRingBuf(const FlexRingBuf& other)
      : FlexRingBuf(
            other,
            alloc_traits::select_on_container_copy_construction(other.alloc_)) {
  }
  /**
   * Allocator-extended copy constructor.
   *
   * @param other The FlexRingBuf to copy values from.
   * @param allocator The allocator to use for storage and element construction.
   * @todo maybe allow other (smaller) sizes as input?
   * @todo Use memcpy/std::copy if Elem is POD
   */
  FlexRingBuf(const FlexRingBuf& other, const allocator_type& allocator)
      : FlexRingBuf(other.capacity(), allocator) {
    clear();

    for (const auto& value : other) {
      push_back(value);
    }
  }
  /**
   * Construct a new FlexRingBuf object out of another, using bulk move
   * assignment.
   *
   * @param other The FlexRingBuf to move the data out of.
   */
  FlexRingBuf(FlexRingBuf&& other) noexcept
      : FlexRingBuf(other.capacity(), std::move(other.get_allocator())) {
    Swap(other);
  }
  /**
   * Allocator-extended move constructor.
   *
   * May move elementwise if the provided allocator and other's allocator are
   * not the same.
   *
   * @param other The FlexRingBuf to move the data out of.
   * @param allocator The allocator to use for storage and element construction.
   */
  FlexRingBuf(FlexRingBuf&& other, const allocator_type& allocator)
      : FlexRingBuf(other.capacity(), allocator) {
    if (other.alloc_ == allocator) {
      Swap(other);
    } else {
      for (auto& element : other) {
        emplace_back(std::move(element));
      }
    }
  }

  /**
   * Copy a FlexRingBuf into this one.
   *
   * First clear()s this FlexRingBuf, and then copies @c other element by
   * element.
   *
   * @param other The FlexRingBuf to copy from.
   * @returns This FlexRingBuf.
   */
  FlexRingBuf& operator=(const FlexRingBuf& other) {
    clear();

    detail::ringbuf::CopyAllocator(alloc_, other.alloc_);

    for (const auto& value : other) {
      push_back(value);
    }
    return *this;
  }
  /**
   * Move a FlexRingBuf into this one.
   *
   * If the allocator is the same or can be moved as well, no elementwise moves
   * are performed.
   *
   * @param other The FlexRingBuf to copy from.
   * @returns This FlexRingBuf.
   */
  FlexRingBuf& operator=(FlexRingBuf&& other) noexcept(
      alloc_traits::propagate_on_container_move_assignment::value ||
      std::is_nothrow_move_constructible<value_type>::value) {
    if (alloc_traits::propagate_on_container_move_assignment::value ||
        alloc_ == other.alloc_) {
      // We're either getting the other's allocator or they're already the same,
      // so swap data in one go.
      detail::ringbuf::MoveAllocator(alloc_, other.alloc_);
      Swap(other);
    } else {
      // Different allocators and can't swap them, so move elementwise.
      clear();
      for (auto& element : other) {
        emplace_back(std::move(element));
      }
    }

    return *this;
  }

  /**
   * Get a copy of the allocator used by this FlexRingBuf.
   */
  allocator_type get_allocator() const { return alloc_; }

  using Base::back;
  using Base::front;

  /**
   * Retrieve an element from the ring buffer without range checking.
   *
   * The behaviour is undefined when @c index is outside [0, size()).
   *
   * @param index The logical index into the ring buffer.
   * @returns A const reference to the element.
   */
  const_reference operator[](const size_type index) const {
    return data_[detail::flexible_ringbuf::RingWrap(capacity_,
                                                    ring_offset_ + index)];
  }
  /**
   * Retrieve an element from the ring buffer without range checking.
   *
   * The behaviour is undefined when @c index is outside [0, size()).
   *
   * @param index The logical index into the ring buffer.
   * @returns A reference to the element.
   */
  reference operator[](const size_type index) {
    return data_[detail::flexible_ringbuf::RingWrap(capacity_,
                                                    ring_offset_ + index)];
  }

  using Base::at;

  /**
   * Get an iterator pointing to the first element.
   */
  iterator begin() noexcept {
    return iterator(&data_[0], capacity_, ring_offset_, 0);
  }
  /**
   * Get an iterator pointing to one past the last element.
   */
  iterator end() noexcept {
    return iterator(&data_[0], capacity_, ring_offset_, size());
  }
  /**
   * Get a const iterator pointing to the first element.
   */
  const_iterator begin() const noexcept {
    return const_iterator(&data_[0], capacity_, ring_offset_, 0);
  }
  /**
   * Get a const iterator pointing to one past the last element.
   */
  const_iterator end() const noexcept {
    return const_iterator(&data_[0], capacity_, ring_offset_, size());
  }
  /**
   * Get a const iterator pointing to the first element.
   */
  const_iterator cbegin() const noexcept { return begin(); }
  /**
   * Get a const iterator pointing to one past the last element.
   */
  const_iterator cend() const noexcept { return end(); }
  /**
   * Get a reverse iterator pointing to the last element.
   */
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  /**
   * Get a reverse iterator pointing to one before the first element.
   */
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  /**
   * Get a const reverse iterator pointing to the last element.
   */
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  /**
   * Get a const reverse iterator pointing to one before the first element.
   */
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  /**
   * Get a const reverse iterator pointing to the last element.
   */
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  /**
   * Get a const reverse iterator pointing to one before the first element.
   */
  const_reverse_iterator crend() const noexcept { return rend(); }

  using Base::empty;

  /**
   * Get the number of elements in the ring buffer.
   */
  size_type size() const noexcept { return size_; }
  /**
   * Get the maximum number of elements in this ring buffer.
   */
  const size_type max_size() const noexcept {
    // Either the numeric limit for end() - begin()
    constexpr auto max_distance =
        std::numeric_limits<difference_type>::max() / sizeof(Elem);
    // .. or the allocator's limit
    // -1, because there's one spare index between end and begin.
    const auto max_alloc = alloc_traits::max_size(alloc_) - 1;
    return std::min(max_distance, max_alloc);
  }
  /**
   * Get the number of elements that can currently be contained without having
   * to drop any.
   */
  size_type capacity() const noexcept { return capacity_; }

  using Base::emplace_back;
  using Base::emplace_front;
  using Base::push_back;
  using Base::push_front;

  /**
   * Pop an element off the front, destroying the first element in the ring
   * buffer.
   */
  void pop_front() noexcept {
    if (empty()) {
      return;
    }

    alloc_traits::destroy(alloc_, &data_[ring_offset_]);
    ShrinkFront();
  }
  /**
   * Pop an element off the back, destroying the last element in the ring
   * buffer.
   */
  void pop_back() noexcept {
    if (empty()) {
      return;
    }

    ShrinkBack();
    alloc_traits::destroy(alloc_, &data_[next_]);
  }

  using Base::clear;

  /**
   * Erase elements in the range [first, last).
   * @param from The first element to erase.
   * @param to One past the last element to erase.
   * @returns Iterator pointing to the element after @c to.
   */
  iterator erase(const_iterator from, const_iterator to) noexcept(
      noexcept(pop_front()) && std::is_nothrow_move_assignable<Elem>::value) {
    if (from == to) {
      return UnConstIterator(to);
    }

    const iterator first = UnConstIterator(from);
    const iterator last = UnConstIterator(to);

    const auto leading = first - begin();
    const auto trailing = end() - last;
    if (leading > trailing) {
      // Move from back towards first
      for (auto i = 0; i < trailing; i++) {
        first[i] = std::move(last[i]);
      }
      const auto to_pop = last - first;
      for (auto i = 0; i < to_pop; i++) {
        pop_back();
      }
    } else {
      // Move from front towards last
      for (auto i = -1; i >= -leading; i--) {
        last[i] = std::move(first[i]);
      }
      const auto to_pop = last - first;
      for (auto i = 0; i < to_pop; i++) {
        pop_front();
      }
    }

    return end() - trailing;
  }

  /**
   * Erase an element.
   *
   * @param pos An iterator pointing to the element to erase.
   * @returns Iterator pointing to the element after @c pos.
   */
  iterator erase(const_iterator pos) noexcept(noexcept(erase(pos, pos + 1))) {
    return erase(pos, pos + 1);
  }

  /**
   * Swap this ring buffer with another using std::swap.
   *
   * @param other The FlexRingBuf to swap with.
   */
  void swap(FlexRingBuf& other) noexcept {
    detail::ringbuf::SwapAllocator(alloc_, other.alloc_);
    Swap(other);
  }
};

/**
 * Copy the elements in the range [@c begin, @c end) to a destination range
 * starting at @c out.
 *
 * Can be used like std::copy:
 * @code
 * std::vector<int> vec;
 * baudvine::copy(ring.begin(), ring.end(), std::back_inserter(vec));
 * @endcode
 *
 * @tparam Ptr The pointer type of the input iterator.
 * @tparam AllocTraits The allocator traits of the input iterator.
 * @tparam capacity_ The capacity of the input iterator.
 * @param begin Start of the source range.
 * @param end End of the source range, one past the last element to copy.
 * @param out Start of the destination range.
 * @returns One past the last copied element in the destination range.
 */
template <typename Ptr, typename AllocTraits, typename OutputIt>
OutputIt copy(const detail::flexible_ringbuf::Iterator<Ptr, AllocTraits>& begin,
              const detail::flexible_ringbuf::Iterator<Ptr, AllocTraits>& end,
              OutputIt out) {
  return detail::flexible_ringbuf::copy(begin, end, out);
}

}  // namespace baudvine
