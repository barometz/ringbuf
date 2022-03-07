#pragma once

#include <baudvine/flexible_ringbuf.h>

template <typename Elem,
          std::size_t Capacity,
          typename Allocator = std::allocator<Elem>>
// Wrapper template to give FlexRingBuf compile-time capacity.
class UnFlexRingBuf : public baudvine::FlexRingBuf<Elem, Allocator> {
 public:
  using Base = baudvine::FlexRingBuf<Elem, Allocator>;
  using typename Base::alloc_traits;
  using typename Base::allocator_type;
  using typename Base::const_iterator;
  using typename Base::const_pointer;
  using typename Base::const_reference;
  using typename Base::const_reverse_iterator;
  using typename Base::difference_type;
  using typename Base::iterator;
  using typename Base::pointer;
  using typename Base::reference;
  using typename Base::reverse_iterator;
  using typename Base::size_type;
  using typename Base::unsigned_difference;
  using typename Base::value_type;

  UnFlexRingBuf() : Base(Capacity) {}
  explicit UnFlexRingBuf(const allocator_type& allocator)
      : Base(Capacity, allocator) {}
  UnFlexRingBuf(const UnFlexRingBuf& other) : Base(other) {}
  UnFlexRingBuf(const UnFlexRingBuf& other, const allocator_type& allocator)
      : Base(other, allocator) {}
  UnFlexRingBuf(UnFlexRingBuf&& other) noexcept : Base(std::move(other)) {}
  UnFlexRingBuf(UnFlexRingBuf&& other, const allocator_type& allocator)
      : Base(std::move(other), allocator) {}

  UnFlexRingBuf& operator=(UnFlexRingBuf&& other) noexcept {
    // TODO: resize
    static_cast<Base&>(*this) = std::move(static_cast<Base&>(other));
    return *this;
  }

  UnFlexRingBuf& operator=(const UnFlexRingBuf& other) {
    static_cast<Base&>(*this) = other;
    return *this;
  }
};
