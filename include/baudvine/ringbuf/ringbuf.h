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
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace baudvine {
namespace detail {

template <
    typename Allocator,
    typename std::enable_if<std::allocator_traits<Allocator>::
                                propagate_on_container_move_assignment::value,
                            int>::type = 0>
void MoveByAssign(Allocator& lhs, Allocator& rhs) noexcept(
    std::is_nothrow_move_assignable<Allocator>::value) {
  lhs = std::move(rhs);
}

template <
    typename Allocator,
    typename std::enable_if<!std::allocator_traits<Allocator>::
                                propagate_on_container_move_assignment::value,
                            int>::type = 0>
void MoveByAssign(Allocator& /*lhs*/, Allocator& /*rhs*/) noexcept {}

template <
    typename Allocator,
    typename std::enable_if<std::allocator_traits<Allocator>::
                                propagate_on_container_move_assignment::value,
                            int>::type = 0>
void MoveBySwap(Allocator& lhs,
                Allocator& rhs) noexcept(noexcept(std::swap(lhs, rhs))) {
  std::swap(lhs, rhs);
}

template <
    typename Allocator,
    typename std::enable_if<!std::allocator_traits<Allocator>::
                                propagate_on_container_move_assignment::value,
                            int>::type = 0>
void MoveBySwap(Allocator& /*lhs*/, Allocator& /*rhs*/) noexcept {}

template <
    typename Allocator,
    typename std::enable_if<
        std::allocator_traits<Allocator>::propagate_on_container_swap::value,
        int>::type = 0>
void SwapAllocator(Allocator& lhs,
                   Allocator& rhs) noexcept(noexcept(std::swap(lhs, rhs))) {
  std::swap(lhs, rhs);
}

template <
    typename Allocator,
    typename std::enable_if<
        !std::allocator_traits<Allocator>::propagate_on_container_swap::value,
        int>::type = 0>
void SwapAllocator(Allocator& /*lhs*/, Allocator& /*rhs*/) noexcept {}

template <
    typename Allocator,
    typename std::enable_if<std::allocator_traits<Allocator>::
                                propagate_on_container_copy_assignment::value,
                            int>::type = 0>
void CopyAllocator(Allocator& lhs, const Allocator& rhs) noexcept(
    std::is_nothrow_copy_assignable<Allocator>::value) {
  lhs = rhs;
}

template <
    typename Allocator,
    typename std::enable_if<!std::allocator_traits<Allocator>::
                                propagate_on_container_copy_assignment::value,
                            int>::type = 0>
void CopyAllocator(Allocator& /*lhs*/, const Allocator& /*rhs*/) noexcept {}

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
  return (ring_index <= Capacity) ? ring_index : ring_index - Capacity - 1;
}

/**
 * @brief An iterator into RingBuf.
 *
 * @tparam Ptr The pointer type, which determines constness.
 * @tparam AllocTraits The allocator traits for the container, used for
 *                     size/difference_type and const_pointer (for auto
 *                     conversion to const iterator).
 * @tparam Capacity The size of the backing array, and maximum size of the ring
 *                  buffer.
 */
template <typename Ptr, typename AllocTraits, size_t Capacity>
class Iterator {
 public:
  using difference_type = typename AllocTraits::difference_type;
  using value_type = typename AllocTraits::value_type;
  using pointer = Ptr;
  using reference = decltype(*pointer{});
  using iterator_category = std::bidirectional_iterator_tag;

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
  operator Iterator<typename AllocTraits::const_pointer,
                    AllocTraits,
                    Capacity>() const {
    return Iterator<typename AllocTraits::const_pointer, AllocTraits, Capacity>(
        data_, ring_offset_, ring_index_);
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

  Iterator operator--(int) noexcept {
    Iterator copy(*this);
    operator--();
    return copy;
  }

  Iterator& operator--() noexcept {
    --ring_index_;
    return *this;
  }

  friend bool operator<(const Iterator& lhs, const Iterator& rhs) noexcept {
    // Comparison via std::tie uses std::tuple::operator<, which compares its
    // elements lexicographically.
    return std::tie(lhs.data_, lhs.ring_offset_, lhs.ring_index_) <
           std::tie(rhs.data_, rhs.ring_offset_, rhs.ring_index_);
  }

  friend bool operator>(const Iterator& lhs, const Iterator& rhs) noexcept {
    return rhs < lhs;
  }

  friend bool operator<=(const Iterator& lhs, const Iterator& rhs) noexcept {
    return !(rhs < lhs);
  }

  friend bool operator>=(const Iterator& lhs, const Iterator& rhs) noexcept {
    return !(lhs < rhs);
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

  template <typename P, typename A, std::size_t C, typename OutputIt>
  // https://github.com/llvm/llvm-project/issues/47430
  // NOLINTNEXTLINE(readability-redundant-declaration)
  friend OutputIt copy(const Iterator<P, A, C>& begin,
                       const Iterator<P, A, C>& end,
                       OutputIt out);

 private:
  pointer data_{};
  // Keeping both ring_offset_ and ring_index_ around is algorithmically
  // redundant (you could add them once and then increment the sum in
  // operator++), but the unchanging ring_offset_ appears to help the compiler
  // optimize RingWrap calls.
  std::size_t ring_offset_{};
  std::size_t ring_index_{};
};

/**
 * @brief Copy the elements in the range [@c begin, @c end) to a destination
 * range starting at @c out.
 *
 * @tparam Ptr The pointer type of the input iterator.
 * @tparam AllocTraits The allocator traits of the input iterator.
 * @tparam Capacity The capacity of the input iterator.
 * @param begin Start of the source range.
 * @param end End of the source range, one past the last element to copy.
 * @param out Start of the destination range.
 * @return OutputIt One past the last copied element in the destination range.
 */
template <typename Ptr,
          typename AllocTraits,
          std::size_t Capacity,
          typename OutputIt>
OutputIt copy(const Iterator<Ptr, AllocTraits, Capacity>& begin,
              const Iterator<Ptr, AllocTraits, Capacity>& end,
              OutputIt out) {
  assert(begin <= end);

  if (begin == end) {
    // Empty range, pass
  } else if (&*end > &*begin) {
    // Fully contiguous range.
    out = std::copy(&*begin, &*end, out);
  } else {
    // Copy in two sections.
    out = std::copy(&*begin, &begin.data_[Capacity + 1], out);
    out = std::copy(end.data_, &*end, out);
  }

  return out;
}

}  // namespace detail

/**
 * @brief The ring buffer itself.
 *
 * @tparam Elem The type of elements contained by the ring buffer.
 * @tparam Capacity The maximum size of the ring buffer, and the fixed size of
 *         the backing array.
 */
template <typename Elem,
          size_t Capacity,
          typename Allocator = std::allocator<Elem>>
class RingBuf {
 public:
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<allocator_type>;
  using value_type = Elem;
  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;
  using reference = decltype(*pointer{});
  using const_reference = decltype(*const_pointer{});
  using iterator = detail::Iterator<pointer, alloc_traits, Capacity>;
  using const_iterator =
      detail::Iterator<const_pointer, alloc_traits, Capacity>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using difference_type = typename alloc_traits::difference_type;
  using size_type = typename alloc_traits::size_type;

  using self = RingBuf<Elem, Capacity>;

 private:
  // The allocator is used to allocate memory, and to construct and destroy
  // elements.
  allocator_type alloc_{};

  // The start of the dynamically allocated backing array.
  pointer data_{nullptr};
  // The next position to write to for push_back().
  size_type next_{0U};

  // Start of the ring buffer in data_.
  size_type ring_offset_{0U};
  // The number of elements in the ring buffer (distance between begin() and
  // end()).
  size_type size_{0U};

  constexpr static size_type Decrement(const size_type index) {
    return index > 0 ? index - 1 : Capacity;
  }

  constexpr static size_type Increment(const size_type index) {
    return index < (Capacity) ? index + 1 : 0;
  }

  // Swap everything but the allocator - caller has to figure that out
  // separately.
  void Swap(RingBuf& other) noexcept {
    std::swap(data_, other.data_);
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
    // Precondition: size != Capacity (when it is, emplace_front pop_backs
    // first.)
    size_++;
  }

  // Move things around after emplace_back.
  void GrowBack() noexcept {
    next_ = Increment(next_);
    // Precondition: size != Capacity (when it is, emplace_back pop_fronts
    // first)
    size_++;
  }

 public:
  /**
   * @brief Construct a new ring buffer object with a default-constructed
   * allocator, and allocate the required memory.
   *
   * Allocates Capacity + 1 to allow for strong exception guarantees in
   * emplace_front/back.
   */
  RingBuf() : RingBuf(allocator_type{}){};
  /**
   * @brief Construct a new ring buffer object with the provided allocator, and
   * allocate the required memory.
   *
   * Allocates Capacity + 1 to allow for strong exception guarantees in
   * emplace_front/back.
   *
   * @param allocator The allocator to use for the backing storage, and
   *                  optionally for element construction and destruction.
   */
  explicit RingBuf(const allocator_type& allocator)
      : alloc_(allocator),
        data_(alloc_traits::allocate(alloc_, Capacity + 1)) {}

  /**
   * @brief Destroy the ring buffer object.
   *
   * Destroys the active elements via clear() and deallocates the backing array.
   */
  ~RingBuf() {
    clear();
    alloc_traits::deallocate(alloc_, data_, Capacity + 1);
  }
  /**
   * @brief Construct a new RingBuf object out of another, using elementwise
   * copy assignment.
   *
   * @param other The RingBuf to copy values from.
   * @todo maybe allow other (smaller) sizes as input?
   */
  RingBuf(const RingBuf& other)
      : RingBuf(
            alloc_traits::select_on_container_copy_construction(other.alloc_)) {
    clear();

    for (const auto& value : other) {
      push_back(value);
    }
  }
  /**
   * @brief Construct a new RingBuf object out of another, using bulk move
   * assignment.
   *
   * @param other The RingBuf to move the data out of.
   * @todo Potential issue when the moved-from RingBuf tries to destroy its
   *       data_ with a moved-from allocator, might need to swap alloc_ instead.
   *       That's not entirely in line with the spec, but safe > correct
   */
  RingBuf(RingBuf&& other) noexcept : RingBuf(std::move(other.alloc_)) {
    Swap(other);
  }

  /**
   * @brief Copy a RingBuf into this one.
   *
   * First clears this RingBuf, and then copies @c other element by element.
   *
   * @param other The RingBuf to copy from.
   * @return This RingBuf.
   */
  RingBuf& operator=(const RingBuf& other) {
    // TODO: copy in bulk when Elem is POD?
    clear();

    detail::CopyAllocator(alloc_, other.alloc_);

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
   * @todo noexcept spec is incomplete
   */
  RingBuf& operator=(RingBuf&& other) noexcept(
      noexcept(detail::MoveByAssign(alloc_, other.alloc_))) {
    if (alloc_traits::propagate_on_container_move_assignment::value ||
        alloc_ == other.alloc_) {
      // We're either getting the other's allocator or they're already the same,
      // so swap data in one go.
      detail::MoveBySwap(alloc_, other.alloc_);
      Swap(other);
    } else {
      // Different allocators and can't swap them, so move elementwise.
      clear();
      detail::MoveByAssign(alloc_, other.alloc_);
      for (auto& element : other) {
        emplace_back(std::move(element));
      }
    }

    return *this;
  }

  allocator_type get_allocator() const { return alloc_; }

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
  const_iterator begin() const noexcept {
    return const_iterator(&data_[0], ring_offset_, 0);
  }
  /**
   * @return A const iterator pointing at one past the last element of the ring
   * buffer.
   */
  const_iterator end() const noexcept {
    return const_iterator(&data_[0], ring_offset_, size());
  }
  /**
   * @return A const iterator pointing at the start of the ring buffer.
   */
  const_iterator cbegin() const noexcept {
    return const_cast<self const&>(*this).begin();
  }
  /**
   * @return A const iterator pointing at one past the last element of the ring
   * buffer.
   */
  const_iterator cend() const noexcept {
    return const_cast<self const&>(*this).end();
  }

  /**
   * @return A reverse iterator pointing at the last element of the ring buffer.
   */
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  /**
   * @return A reverse iterator pointing at one before the beginning of the ring
   * buffer.
   */
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  /**
   * @return A const reverse iterator pointing at the last element of the ring
   * buffer.
   */
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  /**
   * @return A const reverse iterator pointing at one before the first element
   * of the ring buffer.
   */
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  /**
   * @return A const reverse iterator pointing at the last element of the ring
   * buffer.
   */
  const_reverse_iterator crbegin() const noexcept {
    return const_cast<self const&>(*this).rbegin();
  }
  /**
   * @return A const reverse iterator pointing at one before the first element
   * of the ring buffer.
   */
  const_reverse_iterator crend() const noexcept {
    return const_cast<self const&>(*this).rend();
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

    alloc_traits::construct(alloc_, &data_[Decrement(ring_offset_)],
                            std::forward<Args>(args)...);

    // If required, make room for next time.
    if (size() == max_size()) {
      pop_back();
    }
    GrowFront();
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

    alloc_traits::construct(alloc_, &data_[next_], std::forward<Args>(args)...);

    // If required, make room for next time.
    if (size() == max_size()) {
      pop_front();
    }
    GrowBack();
  }

  /**
   * @brief Pop an element off the front, destroying the first element in the
   * ring buffer.
   */
  void pop_front() noexcept {
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
  void pop_back() noexcept {
    if (size() == 0) {
      return;
    }

    ShrinkBack();
    alloc_traits::destroy(alloc_, &data_[next_]);
  }

  /**
   * @brief Remove all elements from the ring buffer, destroying each one
   * starting at the front.
   *
   * After clear(), size() == 0.
   */
  void clear() noexcept(noexcept(pop_front())) {
    // It might be fractionally more efficient to iterate through begin..end and
    // allocator::destroy each one, but this is a lot nicer to read.
    while (!empty()) {
      pop_front();
    }
  }

  /**
   * @brief Swap this ring buffer with another using std::swap.
   *
   * @param other The RingBuf to swap with.
   */
  void swap(RingBuf& other) noexcept(
      noexcept(detail::SwapAllocator(alloc_, other.alloc_))) {
    detail::SwapAllocator(alloc_, other.alloc_);
    Swap(other);
  }

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
};

using detail::copy;

}  // namespace baudvine
