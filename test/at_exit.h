#pragma once

#include <functional>

// Scope-based exit handler for unconditional resource cleanup.
class AtExit {
 public:
  AtExit() = default;
  AtExit(std::function<void()> at_exit) : at_exit_(std::move(at_exit)) {}
  // A complete implementation would probably have working and tested move ops,
  // but this is good enough for the basic case.
  AtExit(const AtExit&) = delete;
  AtExit(AtExit&&) = delete;
  AtExit& operator=(const AtExit&) = delete;
  AtExit& operator=(AtExit&&) = delete;

  ~AtExit() {
    if (at_exit_) {
      at_exit_();
    }
  }

 private:
  std::function<void()> at_exit_;
};
