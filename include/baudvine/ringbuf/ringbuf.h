#pragma once

#include <vector>

namespace baudvine {

template <typename Elem, size_t Size> class ringbuf {
public:
  ringbuf() : Data(Size) {}

  constexpr size_t size() const noexcept { return Size; }
  const Elem &at(size_t index) const { return Data.at(index); }
  Elem &at(size_t index) { return Data.at(index); }

private:
  std::vector<Elem> Data;
};

} // namespace baudvine
