#pragma once

// Utility to track construction and destruction of elements. Each constructor
// increases the counter, and the destructor decrements it.
class InstanceCounter {
 public:
  InstanceCounter();
  InstanceCounter(const InstanceCounter& other);
  InstanceCounter(InstanceCounter&& other) noexcept;
  ~InstanceCounter();

  InstanceCounter& operator=(const InstanceCounter& other);
  InstanceCounter& operator=(InstanceCounter&& other) noexcept;

  static int GetCounter();
  bool IsCopy() const;
  bool IsMoved() const;

 private:
  static int counter_;
  bool copied_{};
  bool moved_{};
};
