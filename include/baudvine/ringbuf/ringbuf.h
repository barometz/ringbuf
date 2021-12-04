#pragma once

#include <vector>

#include <array>

namespace baudvine {

template <typename Elem, size_t Capacity> class ringbuf {
public:
  ringbuf() : Data(Capacity) {}

  constexpr size_t capacity() const noexcept { return Capacity; }
  size_t size() const noexcept { return Size; }
  const Elem &at(size_t index) const {
    if (index >= Size) {
      throw std::out_of_range("ringbuf::at: index >= Size");
    }
    return Data.at((GetBase() + index) % Capacity);
  }
  Elem &at(size_t index) {
    if (index >= Size) {
      throw std::out_of_range("ringbuf::at: index >= Size");
    }
    return Data.at((GetBase() + index) % Capacity);
  }

  void push(const Elem &value) {
    Data.at(Next) = value;

    if (Size < Capacity)
      Size++;

    Next = (Next + 1) % Capacity;
  }

private:
  std::vector<Elem> Data;
  size_t Next{0U};
  size_t Size{0U};

  size_t GetBase() const noexcept {
    if (Size == Capacity)
      return Next;
    else
      return 0;
  }
};

} // namespace baudvine
