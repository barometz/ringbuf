#pragma once

// Utility to track construction and destruction of elements. Each constructor
// increases the counter, and the destructor decrements it.
class InstanceCounter {
 public:
  InstanceCounter();
  InstanceCounter(const InstanceCounter& other);
  InstanceCounter(InstanceCounter&& other) noexcept;
  ~InstanceCounter();

  static int GetCounter();

private:
  static int counter_;
};
