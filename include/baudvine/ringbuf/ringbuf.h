#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <tuple>

namespace baudvine {
namespace detail {

// TODO: reverse iterator
// TODO: clear
// TODO: erase?
// TODO: front, back

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
  using pointer = Elem*;
  using const_reference = const Elem&;
  using iterator = detail::Iterator<Elem, Capacity>;
  using const_iterator = detail::ConstIterator<Elem, Capacity>;
  using difference_type = typename iterator::difference_type;
  using size_type = std::size_t;
  using alloc = std::allocator<value_type>;
  using allocator_traits = std::allocator_traits<alloc>;

  RingBuf() : data_(allocator_traits::allocate(allocator_, Capacity)){};
  ~RingBuf() {
    while (!empty()) {
      pop_front();
    }
    allocator_traits::deallocate(allocator_, data_, Capacity);
  }

  RingBuf(const RingBuf& other)
      : data_(allocator_traits::allocate(allocator_, Capacity)) {
    *this = other;
  }

  RingBuf(RingBuf&& other)
      : data_(allocator_traits::allocate(allocator_, Capacity)) {
    *this = std::move(other);
  }

  RingBuf& operator=(const RingBuf& other) {
    while (!empty()) {
      pop_front();
    }
    for (const auto& value : other) {
      push_back(value);
    }
    return *this;
  }
  RingBuf& operator=(RingBuf&& other) {
    while (!empty()) {
      pop_front();
    }
    for (const auto& value : other) {
      push_back(std::move(value));
    }
    return *this;
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

  iterator begin() noexcept {
    return iterator(&this->data_[0], this->base_, 0);
  }
  iterator end() noexcept {
    return iterator(&this->data_[0], this->base_, this->size());
  }
  const_iterator begin() const noexcept { return cbegin(); }
  const_iterator end() const noexcept { return cend(); }
  const_iterator cbegin() const noexcept {
    return const_iterator(&this->data_[0], this->base_, 0);
  }
  const_iterator cend() const noexcept {
    return const_iterator(&this->data_[0], this->base_, this->size());
  }

  bool empty() const noexcept { return this->size() == 0; }
  size_type size() const noexcept { return size_; }
  constexpr size_type max_size() const noexcept { return Capacity; }
  constexpr size_type capacity() const noexcept { return Capacity; }

  void push_back(const_reference value) {
    if (Capacity == 0) {
      // A buffer of size zero is conceptually sound, so let's support it.
      return;
    }

    if (size_ == Capacity) {
      pop_front();
    }

    allocator_traits::construct(allocator_, &data_[GetNext()], value);
    Progress();
  }

  void push_back(value_type&& value) { return emplace_back(std::move(value)); }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    if (Capacity == 0) {
      // A buffer of size zero is conceptually sound, so let's support it.
      return;
    }

    if (size_ == Capacity) {
      pop_front();
    }

    allocator_traits::construct(allocator_, &data_[GetNext()],
                                std::forward<Args>(args)...);
    Progress();
  }

  void pop_front() {
    if (size_ == 0) {
      return;
    }

    allocator_traits::destroy(allocator_, &data_[base_]);

    base_ = detail::RingWrap<Capacity>(base_ + 1);
    size_--;
  }

  void swap(RingBuf& other) { std::swap(*this, other); }

  friend bool operator<(const RingBuf& lhs, const RingBuf& rhs) {
    if (lhs.size() != rhs.size()) {
      return false;
    }

    auto end = lhs.cend();
    auto mismatch = std::mismatch(lhs.cbegin(), end, rhs.cbegin());
    if (mismatch.first == end) {
      return false;
    }

    return *mismatch.first < *mismatch.second;
  }

  friend bool operator>(const RingBuf& lhs, const RingBuf& rhs) {
    if (lhs.size() != rhs.size()) {
      return false;
    }

    auto end = lhs.cend();
    auto mismatch = std::mismatch(lhs.cbegin(), end, rhs.cbegin());
    if (mismatch.first == end) {
      return false;
    }

    return *mismatch.first < *mismatch.second;
  }

  friend bool operator==(const RingBuf& lhs, const RingBuf& rhs) {
    if (lhs.size() != rhs.size()) {
      return false;
    }

    auto end = lhs.cend();
    auto mismatch = std::mismatch(lhs.cbegin(), end, rhs.cbegin());
    return mismatch.first == end;
  }

  friend bool operator>=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs < rhs);
  }
  friend bool operator<=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs > rhs);
  }
  friend bool operator!=(const RingBuf& lhs, const RingBuf& rhs) {
    return !(lhs == rhs);
  }

 private:
  alloc allocator_{};
  pointer data_{nullptr};
  size_type base_{0U};
  size_type size_{0U};

  size_type GetNext() { return detail::RingWrap<Capacity>(base_ + size_); }

  // Move things around after growing.
  void Progress() {
    // The base only moves when we're full.
    if (size_ == Capacity) {
      base_ = detail::RingWrap<Capacity>(base_ + 1);
    }

    // Size will never exceed the capacity.
    if (size_ < Capacity) {
      size_++;
    }
  }
};
}  // namespace baudvine
