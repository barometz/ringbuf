#pragma once

#include <cstddef>
#include <deque>

namespace baudvine {

// Container with ring buffer semantics, backed by non-contiguous dynamically
// allocated memory.
template <typename Elem>
class DynamicRingBuf {
 public:
  using value_type = Elem;
  using reference = Elem&;
  using const_reference = const Elem&;
  using iterator = typename std::deque<value_type>::iterator;
  using const_iterator = typename std::deque<value_type>::const_iterator;
  using difference_type = typename iterator::difference_type;
  using size_type = std::size_t;

  DynamicRingBuf() {}
  DynamicRingBuf(size_type capacity) : capacity_(capacity) {}

  size_type capacity() const { return capacity_; }
  size_type size() const { return data_.size(); }
  bool empty() const { return data_.empty(); }

  void set_capacity(size_type capacity) {
    capacity_ = capacity;
    while (size() > capacity_) {
      pop_front();
    }
  }

  void push_back(const_reference value) {
    if (size() == capacity()) {
      pop_front();
    }

    data_.push_back(value);
  }

  void pop_front() { data_.pop_front(); }

  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }
  const_iterator cbegin() const { return data_.cbegin(); }
  const_iterator cend() const { return data_.cend(); }

  reference operator[](size_type index) { return data_[index]; }
  const_reference operator[](size_type index) const { return data_[index]; }
  reference at(size_type index) { return data_.at(index); }
  const_reference at(size_type index) const { return data_.at(index); }

 private:
  std::deque<value_type> data_{};
  size_type capacity_{};
};

}  // namespace baudvine
