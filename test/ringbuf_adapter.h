#pragma once

#include <baudvine/ringbuf/deque_ringbuf.h>
#include <baudvine/ringbuf/ringbuf.h>

#include <ostream>

enum class Variant {
  Static,
  Dynamic,
};

inline std::ostream& operator<<(std::ostream& os, Variant variant) {
  switch (variant) {
    case Variant::Static:
      return os << "Static";
    case Variant::Dynamic:
      return os << "Dynamic";
  }
}

// Adapter for the two ringbuf implementations, so one set of tests can test
// both (except for iterators). There's probably room for more type erasure, but
// this works fine.
template <typename Elem, std::size_t Capacity>
class RingBufAdapter {
 public:
  RingBufAdapter(Variant variant) : variant_(variant) {
    switch (variant) {
      case Variant::Static:
        static_ = baudvine::RingBuf<Elem, Capacity>();
        break;
      case Variant::Dynamic:
        dynamic_ = baudvine::DequeRingBuf<Elem>(Capacity);
        break;
    }
  }

#define DISPATCH(call)         \
  do {                         \
    switch (variant_) {        \
      case (Variant::Static):  \
        return static_.call;   \
      case (Variant::Dynamic): \
        return dynamic_.call;  \
    }                          \
  } while (false)

  Elem& front() { DISPATCH(front()); }
  Elem& back() { DISPATCH(back()); }
  Elem& operator[](size_t index) { DISPATCH(operator[](index)); }
  const Elem& operator[](size_t index) const { DISPATCH(operator[](index)); }
  Elem& at(size_t index) { DISPATCH(at(index)); }
  const Elem& at(size_t index) const { DISPATCH(at(index)); }
  void clear() { DISPATCH(clear()); }
  void push_back(const Elem& value) { DISPATCH(push_back(value)); }
  void push_back(Elem&& value) { DISPATCH(push_back(std::move(value))); }
  template <typename... Args>
  void emplace_back(Args... args) {
    DISPATCH(emplace_back(std::forward<Args>(args)...));
  }
  void pop_front() { DISPATCH(pop_front()); }
  size_t size() { DISPATCH(size()); }
  size_t capacity() { DISPATCH(capacity()); }
  size_t max_size() { DISPATCH(max_size()); }
  bool empty() { DISPATCH(empty()); }

  friend bool operator<(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    switch (lhs.variant_) {
      case (Variant::Static):
        return lhs.static_ < rhs.static_;
      case (Variant::Dynamic):
        return lhs.dynamic_ < rhs.dynamic_;
    }
  }

  friend bool operator==(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    switch (lhs.variant_) {
      case (Variant::Static):
        return lhs.static_ == rhs.static_;
      case (Variant::Dynamic):
        return lhs.dynamic_ == rhs.dynamic_;
    }
  }

  friend bool operator!=(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    switch (lhs.variant_) {
      case (Variant::Static):
        return lhs.static_ != rhs.static_;
      case (Variant::Dynamic):
        return lhs.dynamic_ != rhs.dynamic_;
    }
  }

  void swap(RingBufAdapter& other) {
    switch (variant_) {
      case (Variant::Static):
        return static_.swap(other.static_);
      case (Variant::Dynamic):
        return dynamic_.swap(other.dynamic_);
    }
  }

#undef DISPATCH

 private:
  Variant variant_;

  // This would be less annoying with std::variant, but the build should also
  // run with C++11.
  baudvine::RingBuf<Elem, Capacity> static_{};
  baudvine::DequeRingBuf<Elem> dynamic_{};
};
