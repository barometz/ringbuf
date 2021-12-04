#pragma once

#include <cassert>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace baudvine {
namespace detail {

// TODO: move all public functions of RingBufBase over to RingBuf
// TODO: make sure moving out of RingBuf leaves something sound
// TODO: move params, emplace smarts
// TODO: reverse iterator
// TODO: decide between front/at/??

template <typename Elem, std::size_t Capacity>
class RingBufBase {
 public:
  using value_type = Elem;
  using reference = Elem&;
  using size_type = std::size_t;

  RingBufBase() : data_(Capacity) {}

  constexpr size_type capacity() const noexcept { return Capacity; }
  size_type max_size() const noexcept { return this->data_.max_size(); }
  size_type size() const noexcept { return size_; }

  const Elem& operator[](size_type index) const {
    return data_[(base_ + index) % Capacity];
  }

  Elem& operator[](size_type index) { return data_[(base_ + index) % Capacity]; }

  const Elem& at(size_type index) const {
    if (index >= size_) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return (*this)[index];
  }
  Elem& at(size_type index) {
    if (index >= size_) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return (*this)[index];
  }

  void push_back(const Elem& value) {
    if (Capacity == 0) {
      // A buffer of size zero is conceptually sound, so let's support it.
      return;
    }

    data_[GetNext()] = value;

    if (size_ == Capacity) {
      base_ = (base_ + 1) % Capacity;
    }

    if (size_ < Capacity) {
      size_++;
    }
  }

  void pop_front() {
    if (size_ == 0) {
      return;
    }

    base_ = (base_ + 1) % Capacity;
    size_--;
  }

 private:
  std::vector<Elem> data_;
  size_type base_{0U};
  size_type size_{0U};

  size_type GetNext() { return (base_ + size_) % Capacity; }
};

template <typename Elem, std::size_t Capacity>
class ConstIterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Elem;
  using pointer = const Elem*;
  using reference = const Elem&;
  using iterator_category = std::forward_iterator_tag;

  constexpr ConstIterator() noexcept {}
  ConstIterator(const RingBufBase<Elem, Capacity>& ring_buf,
                std::size_t position) noexcept
      : ring_buf_(&ring_buf), position_(position) {}

  reference operator*() const { return (*ring_buf_)[position_]; }

  ConstIterator operator++(int) noexcept {
    ConstIterator copy(*this);
    operator++();
    return copy;
  }

  ConstIterator& operator++() noexcept {
    position_ = std::min(position_ + 1, ring_buf_->size());
    return *this;
  }

  friend bool operator<(const ConstIterator& lhs,
                        const ConstIterator& rhs) noexcept {
    return std::tie(lhs.ring_buf_, lhs.position_) <
           std::tie(rhs.ring_buf_, rhs.position_);
  }

  friend bool operator==(const ConstIterator& lhs,
                         const ConstIterator& rhs) noexcept {
    return std::tie(lhs.ring_buf_, lhs.position_) ==
           std::tie(rhs.ring_buf_, rhs.position_);
  }

  friend bool operator!=(const ConstIterator& lhs,
                         const ConstIterator& rhs) noexcept {
    return !(lhs == rhs);
  }

 private:
  const RingBufBase<Elem, Capacity>* ring_buf_{};
  size_t position_{};
};

template <typename Elem, size_t Capacity>
class Iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Elem;
  using pointer = Elem*;
  using reference = Elem&;
  using iterator_category = std::forward_iterator_tag;

  Iterator() {}
  Iterator(RingBufBase<Elem, Capacity>& ring_buf, size_t position)
      : ring_buf_(&ring_buf), position_(position) {}

  operator ConstIterator<Elem, Capacity>() const {
    return ConstIterator<Elem, Capacity>(*ring_buf_, position_);
  }

  reference operator*() const { return (*ring_buf_)[position_]; }

  Iterator operator++(int) noexcept {
    Iterator copy(*this);
    operator++();
    return copy;
  }

  Iterator& operator++() noexcept {
    position_ = std::min(position_ + 1, ring_buf_->size());
    return *this;
  }

  friend bool operator<(const Iterator& lhs, const Iterator& rhs) noexcept {
    return std::tie(lhs.ring_buf_, lhs.position_) <
           std::tie(rhs.ring_buf_, rhs.position_);
  }

  friend bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept {
    return std::tie(lhs.ring_buf_, lhs.position_) ==
           std::tie(rhs.ring_buf_, rhs.position_);
  }

  friend bool operator!=(const Iterator& lhs, const Iterator& rhs) noexcept {
    return !(lhs == rhs);
  }

 private:
  RingBufBase<Elem, Capacity>* ring_buf_{};
  size_t position_{};
};

}  // namespace detail

template <typename Elem, size_t Capacity>
class RingBuf : public detail::RingBufBase<Elem, Capacity> {
  using Self = RingBuf<Elem, Capacity>;
  using Base = detail::RingBufBase<Elem, Capacity>;

 public:
  using size_type = typename Base::size_type;
  using iterator = detail::Iterator<Elem, Capacity>;
  using const_iterator = detail::ConstIterator<Elem, Capacity>;
  using difference_type = typename iterator::difference_type;

  bool empty() const noexcept { return this->size() == 0; }

  iterator begin() noexcept { return iterator(*this, 0); }
  iterator end() noexcept { return iterator(*this, this->size()); }
  const_iterator cbegin() const noexcept { return const_iterator(*this, 0); }
  const_iterator cend() const noexcept {
    return const_iterator(*this, this->size());
  }

  void swap(Self& other) { std::swap(*this, other); }

  friend bool operator==(const Self& lhs, const Self& rhs) {
    if (lhs.size() != rhs.size())
      return false;

    auto end = lhs.cend();
    auto mismatch = std::mismatch(lhs.cbegin(), end, rhs.cbegin());
    return mismatch.first == end;
  }

  friend bool operator!=(const Self& lhs, const Self& rhs) {
    return !(lhs == rhs);
  }
};

}  // namespace baudvine
