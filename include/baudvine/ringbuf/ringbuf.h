#pragma once

namespace baudvine {

template <typename Elem, size_t Size> class ringbuf {
public:
  constexpr size_t size() const noexcept { return Size; }
};

} // namespace baudvine
