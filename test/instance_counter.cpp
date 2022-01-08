#include "instance_counter.h"

int InstanceCounter::counter_ = 0;

InstanceCounter::InstanceCounter() {
  counter_++;
}

InstanceCounter::InstanceCounter(const InstanceCounter& /*other*/)
    : copied_(true) {
  counter_++;
}

InstanceCounter::InstanceCounter(InstanceCounter&& /*other*/) noexcept
    : moved_(true) {
  counter_++;
}

InstanceCounter::~InstanceCounter() {
  counter_--;
}

InstanceCounter& InstanceCounter::operator=(const InstanceCounter& /*other*/) {
  copied_ = true;
  return *this;
}
InstanceCounter& InstanceCounter::operator=(
    InstanceCounter&& /*other*/) noexcept {
  moved_ = true;
  return *this;
}

int InstanceCounter::GetCounter() {
  return counter_;
}

bool InstanceCounter::IsCopy() const {
  return copied_;
}

bool InstanceCounter::IsMoved() const {
  return moved_;
}
