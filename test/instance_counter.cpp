#include "instance_counter.h"

int InstanceCounter::counter_ = 0;

InstanceCounter::InstanceCounter() {
  counter_++;
}
InstanceCounter::InstanceCounter(const InstanceCounter& /*other*/) {
  counter_++;
}
InstanceCounter::InstanceCounter(InstanceCounter&& /*other*/) noexcept {
  counter_++;
}
InstanceCounter::~InstanceCounter() {
  counter_--;
}

int InstanceCounter::GetCounter() {
  return counter_;
}
