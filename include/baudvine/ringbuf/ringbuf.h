#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <tuple>

namespace baudvine {
namespace detail {

// TODO: move params, emplace smarts
// TODO: reverse iterator
// TODO: std::copy optimization?
// TODO: begin() const

// Would-be-nices:
// - std::array-style aggregate initialization. Probably impossible because
//   RingBuf has private fields.
// - Do something smart with in-place construction/destruction

template <std::size_t Capacity>
constexpr std::size_t RingWrap(std::size_t position) {
  return position % Capacity;
}
template <>
constexpr std::size_t RingWrap<0>(std::size_t /*position*/) {
  return 0;
}

template <typename Elem, std::size_t Capacity>
class ConstIterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Elem;
  using pointer = const Elem*;
  using reference = const Elem&;
  using iterator_category = std::forward_iterator_tag;

  constexpr ConstIterator() noexcept {}
  ConstIterator(pointer data, std::size_t base, std::size_t position) noexcept
      : data_(data), base_(base), position_(position) {}

  reference operator*() const {
    return data_[RingWrap<Capacity>(base_ + position_)];
  }

  ConstIterator operator++(int) noexcept {
    ConstIterator copy(*this);
    operator++();
    return copy;
  }

  ConstIterator& operator++() noexcept {
    ++position_;
    return *this;
  }

  friend bool operator<(const ConstIterator& lhs,
                        const ConstIterator& rhs) noexcept {
    return std::tie(lhs.data_, lhs.base_, lhs.position_) <
           std::tie(rhs.data_, rhs.base_, rhs.position_);
  }

  friend bool operator==(const ConstIterator& lhs,
                         const ConstIterator& rhs) noexcept {
    return std::tie(lhs.data_, lhs.base_, lhs.position_) ==
           std::tie(rhs.data_, rhs.base_, rhs.position_);
  }

  friend bool operator!=(const ConstIterator& lhs,
                         const ConstIterator& rhs) noexcept {
    return !(lhs == rhs);
  }

 private:
  pointer data_{};
  std::size_t base_{};
  std::size_t position_{};
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
  Iterator(pointer data, std::size_t base, size_t position)
      : data_(data), base_(base), position_(position) {}

  operator ConstIterator<value_type, Capacity>() const {
    return ConstIterator<value_type, Capacity>(data_, base_, position_);
  }

  reference operator*() const {
    return data_[RingWrap<Capacity>(base_ + position_)];
  }

  pointer operator->() const { return &**this; }

  Iterator operator++(int) noexcept {
    Iterator copy(*this);
    operator++();
    return copy;
  }

  Iterator& operator++() noexcept {
    ++position_;
    return *this;
  }

  friend bool operator<(const Iterator& lhs, const Iterator& rhs) noexcept {
    return std::tie(lhs.data_, lhs.base_, lhs.position_) <
           std::tie(rhs.data_, rhs.base_, rhs.position_);
  }

  friend bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept {
    return std::tie(lhs.data_, lhs.base_, lhs.position_) ==
           std::tie(rhs.data_, rhs.base_, rhs.position_);
  }

  friend bool operator!=(const Iterator& lhs, const Iterator& rhs) noexcept {
    return !(lhs == rhs);
  }

 private:
  pointer data_{};
  std::size_t base_{};
  std::size_t position_{};
};

}  // namespace detail

template <typename Elem, size_t Capacity>
class RingBuf {
 public:
  using value_type = Elem;
  using reference = Elem&;
  using const_reference = const Elem&;
  using iterator = detail::Iterator<Elem, Capacity>;
  using const_iterator = detail::ConstIterator<Elem, Capacity>;
  using difference_type = typename iterator::difference_type;
  using size_type = std::size_t;

  constexpr size_type max_size() const noexcept { return Capacity; }
  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return this->size() == 0; }

  void push_back(const_reference value) {
    if (Capacity == 0) {
      // A buffer of size zero is conceptually sound, so let's support it.
      return;
    }

    data_[GetNext()] = value;

    // The base only moves when we're full.
    if (size_ == Capacity) {
      base_ = detail::RingWrap<Capacity>(base_ + 1);
    }

    // Size will never exceed the capacity.
    if (size_ < Capacity) {
      size_++;
    }
  }

  void pop_front() {
    if (size_ == 0) {
      return;
    }

    data_[base_] = {};
    base_ = detail::RingWrap<Capacity>(base_ + 1);
    size_--;
  }

  iterator begin() noexcept {
    return iterator(&this->data_[0], this->base_, 0);
  }
  iterator end() noexcept {
    return iterator(&this->data_[0], this->base_, this->size());
  }
  const_iterator cbegin() const noexcept {
    return const_iterator(&this->data_[0], this->base_, 0);
  }
  const_iterator cend() const noexcept {
    return const_iterator(&this->data_[0], this->base_, this->size());
  }

  const_reference operator[](size_type index) const {
    return data_[detail::RingWrap<Capacity>(base_ + index)];
  }
  reference operator[](size_type index) {
    return data_[detail::RingWrap<Capacity>(base_ + index)];
  }

  const_reference at(size_type index) const {
    if (index >= this->size()) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return (*this)[index];
  }
  reference at(size_type index) {
    if (index >= this->size()) {
      throw std::out_of_range("RingBuf::at: index >= Size");
    }
    return (*this)[index];
  }

  void swap(RingBuf& other) { std::swap(*this, other); }

  friend bool operator==(const RingBuf& lhs, const RingBuf& rhs) {
    if (lhs.size() != rhs.size())
      return false;

    auto end = lhs.cend();
    auto mismatch = std::mismatch(lhs.cbegin(), end, rhs.cbegin());
    return mismatch.first == end;
  }

  friend bool operator!=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs == rhs);
  }

 private:
  value_type data_[Capacity]{};
  size_type base_{0U};
  size_type size_{0U};

  size_type GetNext() { return detail::RingWrap<Capacity>(base_ + size_); }
};
}  // namespace baudvine
