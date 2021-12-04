#pragma once

#include <vector>

#include <array>

namespace baudvine {

namespace detail {

template <typename Elem, size_t MaxSize>
class RingBufBase {
 public:
  RingBufBase() : data_(MaxSize) {}

  size_t Size() const noexcept { return size_; }

  constexpr size_t Capacity() const noexcept { return MaxSize; }
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

 protected:
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

template <typename Elem, size_t MaxSize>
class Iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Elem;
  using pointer = Elem*;
  using reference = Elem&;
  using iterator_category = std::forward_iterator_tag;

  Iterator() {}
  Iterator(RingBufBase<Elem, MaxSize>& ring_buf, size_t position)
      : ring_buf_(&ring_buf), position_(position) {}

  reference operator*() { return ring_buf_->At(position_); }

  Iterator operator++(int) {
    Iterator copy(*this);
    operator++();
    return copy;
  }

  Iterator& operator++() {
    position_ = std::min(position_ + 1, ring_buf_->Size());
    return *this;
  }

  friend bool operator<(const Iterator& lhs, const Iterator& rhs) {
    return std::tie(lhs.ring_buf_, lhs.position_) <
           std::tie(rhs.ring_buf_, rhs.position_);
  }

  friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
    return std::tie(lhs.ring_buf_, lhs.position_) ==
           std::tie(rhs.ring_buf_, rhs.position_);
  }

  friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
    return !(lhs == rhs);
  }

 private:
  RingBufBase<Elem, MaxSize>* ring_buf_{};
  size_t position_{};
};

}  // namespace detail

template <typename Elem, size_t MaxSize>
class RingBuf : public detail::RingBufBase<Elem, MaxSize> {
 public:
  using value_type = Elem;
  using reference = Elem&;
  using const_reference = const Elem&;
  using iterator = detail::Iterator<Elem, MaxSize>;
  // TODO const_iterator =
  using difference_type = typename iterator::difference_type;
  using size_type = size_t;

  iterator begin() { return iterator(*this, 0); }
  iterator end() { return iterator(*this, this->Size()); }
};

}  // namespace baudvine
