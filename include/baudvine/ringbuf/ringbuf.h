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
// TODO: type erase for iterators - would be nice if they didn't have to be tied
//       to a buffer size

template <typename Elem, std::size_t MaxSize>
class RingBufBase {
 public:
  using value_type = Elem;
  using reference = Elem&;
  using size_type = std::size_t;

  RingBufBase() : data_(MaxSize) {}

  size_type Size() const noexcept { return size_; }

  constexpr size_type Capacity() const noexcept { return MaxSize; }
  const Elem& At(size_type index) const {
    if (index >= size_) {
      throw std::out_of_range("RingBuf::At: index >= Size");
    }
    return data_.at((GetBase() + index) % Capacity());
  }
  Elem& At(size_type index) {
    if (index >= size_) {
      throw std::out_of_range("RingBuf::At: index >= Size");
    }
    return data_.at((GetBase() + index) % Capacity());
  }

  void Push(const Elem& value) {
    if (!Capacity()) {
      // A buffer of size zero is conceptually sound, so let's support it.
      return;
    }

    data_.at(next_) = value;

    if (size_ < Capacity()) {
      size_++;
    }

    next_ = (next_ + 1) % Capacity();
  }

 protected:
  std::vector<Elem> data_;
  size_type next_{0U};
  size_type size_{0U};

  size_type GetBase() const noexcept {
    if (size_ == Capacity()) {
      return next_;
    } else {
      return 0;
    }
  }
};

template <typename Elem, std::size_t MaxSize>
class ConstIterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Elem;
  using pointer = const Elem*;
  using reference = const Elem&;
  using iterator_category = std::forward_iterator_tag;

  constexpr ConstIterator() noexcept {}
  ConstIterator(const RingBufBase<Elem, MaxSize>& ring_buf,
                std::size_t position) noexcept
      : ring_buf_(&ring_buf), position_(position) {}

  reference operator*() const { return ring_buf_->At(position_); }

  ConstIterator operator++(int) noexcept {
    ConstIterator copy(*this);
    operator++();
    return copy;
  }

  ConstIterator& operator++() noexcept {
    position_ = std::min(position_ + 1, ring_buf_->Size());
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
  const RingBufBase<Elem, MaxSize>* ring_buf_{};
  size_t position_{};
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

  operator ConstIterator<Elem, MaxSize>() const {
    return ConstIterator<Elem, MaxSize>(*ring_buf_, position_);
  }

  reference operator*() const { return ring_buf_->At(position_); }

  Iterator operator++(int) noexcept {
    Iterator copy(*this);
    operator++();
    return copy;
  }

  Iterator& operator++() noexcept {
    position_ = std::min(position_ + 1, ring_buf_->Size());
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
  RingBufBase<Elem, MaxSize>* ring_buf_{};
  size_t position_{};
};

}  // namespace detail

template <typename Elem, size_t MaxSize>
class RingBuf : public detail::RingBufBase<Elem, MaxSize> {
  using Self = RingBuf<Elem, MaxSize>;
  using Base = detail::RingBufBase<Elem, MaxSize>;

 public:
  using size_type = typename Base::size_type;
  using iterator = detail::Iterator<Elem, MaxSize>;
  using const_iterator = detail::ConstIterator<Elem, MaxSize>;
  using difference_type = typename iterator::difference_type;

  size_type size() const noexcept { return this->Size(); }
  size_type max_size() const noexcept { return this->data_.max_size(); }
  bool empty() const noexcept { return this->Size() == 0; }

  iterator begin() noexcept { return iterator(*this, 0); }
  iterator end() noexcept { return iterator(*this, this->Size()); }
  const_iterator cbegin() const noexcept { return const_iterator(*this, 0); }
  const_iterator cend() const noexcept {
    return const_iterator(*this, this->Size());
  }

  void swap(Self& other) { std::swap(*this, other); }

  friend bool operator==(const Self& lhs, const Self& rhs) {
    if (lhs.Size() != rhs.Size())
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
