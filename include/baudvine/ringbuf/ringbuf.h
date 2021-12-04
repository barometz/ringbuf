#pragma once

#include <vector>

#include <array>

namespace baudvine {

template <typename Elem, size_t MaxSize>
class RingBuf {
 public:
  RingBuf() : data_(MaxSize) {}

  constexpr size_t Capacity() const noexcept { return MaxSize; }
  size_t Size() const noexcept { return size_; }
  const Elem& At(size_t index) const {
    if (index >= size_) {
      throw std::out_of_range("RingBuf::At: index >= Size");
    }
    return data_.at((GetBase() + index) % Capacity());
  }
  Elem& At(size_t index) {
    if (index >= size_) {
      throw std::out_of_range("RingBuf::At: index >= Size");
    }
    return data_.at((GetBase() + index) % Capacity());
  }

  void Push(const Elem& value) {
    data_.at(next_) = value;

    if (size_ < Capacity())
      size_++;

    next_ = (next_ + 1) % Capacity();
  }

 private:
  std::vector<Elem> data_;
  size_t next_{0U};
  size_t size_{0U};

  size_t GetBase() const noexcept {
    if (size_ == Capacity())
      return next_;
    else
      return 0;
  }
};

}  // namespace baudvine
