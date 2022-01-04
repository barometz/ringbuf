#pragma once

#include "config.h"

#ifdef BAUDVINE_HAVE_VARIANT
#define BAUDVINE_HAVE_RINGBUF_ADAPTER

#include <baudvine/ringbuf/deque_ringbuf.h>
#include <baudvine/ringbuf/ringbuf.h>

#include <ostream>
#include <variant>

enum class Variant {
  Standard,
  Deque,
};

inline std::ostream& operator<<(std::ostream& os, Variant variant) {
  return os << (variant == Variant::Standard ? "Standard" : "Deque");
}

// Adapter for the two ringbuf implementations, so one set of tests can test
// both (except for iterators). There's probably room for more type erasure, but
// this works fine.
template <typename Elem, std::size_t Capacity>
class RingBufAdapter {
 public:
  RingBufAdapter(Variant variant) {
    switch (variant) {
      case Variant::Standard:
        ringbuf_.template emplace<baudvine::RingBuf<Elem, Capacity>>();
        break;
      case Variant::Deque:
        ringbuf_.template emplace<baudvine::DequeRingBuf<Elem, Capacity>>();
        break;
    }
  }

#define DISPATCH(call) \
  std::visit([&](auto&& b) -> decltype(auto) { return b.call; }, ringbuf_)

  Elem& front() { return DISPATCH(front()); }
  Elem& back() { return DISPATCH(back()); }
  Elem& operator[](size_t index) { return DISPATCH(operator[](index)); }
  const Elem& operator[](size_t index) const {
    return DISPATCH(operator[](index));
  }
  Elem& at(size_t index) { return DISPATCH(at(index)); }
  const Elem& at(size_t index) const { return DISPATCH(at(index)); }
  void clear() { return DISPATCH(clear()); }
  void push_front(const Elem& value) { return DISPATCH(push_front(value)); }
  void push_front(Elem&& value) {
    return DISPATCH(push_front(std::move(value)));
  }
  template <typename... Args>
  void emplace_front(Args... args) {
    return DISPATCH(emplace_front(std::forward<Args>(args)...));
  }
  void push_back(const Elem& value) { return DISPATCH(push_back(value)); }
  void push_back(Elem&& value) { return DISPATCH(push_back(std::move(value))); }
  template <typename... Args>
  void emplace_back(Args... args) {
    return DISPATCH(emplace_back(std::forward<Args>(args)...));
  }
  void pop_front() { return DISPATCH(pop_front()); }
  void pop_back() { return DISPATCH(pop_back()); }
  size_t size() { return DISPATCH(size()); }
  size_t max_size() { return DISPATCH(max_size()); }
  bool empty() { return DISPATCH(empty()); }

  friend bool operator<(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    return lhs.ringbuf_ < rhs.ringbuf_;
  }

  friend bool operator==(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    return lhs.ringbuf_ == rhs.ringbuf_;
  }

  friend bool operator!=(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    return lhs.ringbuf_ != rhs.ringbuf_;
  }

  void swap(RingBufAdapter& other) { return ringbuf_.swap(other.ringbuf_); }

#undef DISPATCH

 private:
  std::variant<baudvine::RingBuf<Elem, Capacity>,
               baudvine::DequeRingBuf<Elem, Capacity>>
      ringbuf_;
};

#endif  // BAUDVINE_HAVE_VARIANT
