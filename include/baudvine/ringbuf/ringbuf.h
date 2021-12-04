#pragma once

#include <vector>

#include <array>

namespace baudvine {

template <typename Elem, size_t Capacity> class ringbuf {
public:
  ringbuf() : Data(Capacity) {}

  // TODO hang on, size and capacity aren't the same
  constexpr size_t capacity() const noexcept { return Capacity; }
  const Elem &at(size_t index) const { return Data.at(index); }
  Elem &at(size_t index) { return Data.at(index); }

  // void push(const T& value) { }

private:
  std::vector<Elem> Data;
  size_t Size{0U};
};

} // namespace baudvine
