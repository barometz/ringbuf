#include <baudvine/ringbuf/deque_ringbuf.h>
#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>

#include <chrono>

// Speed comparison between deque and standard. Standard should at least be as
// fast as deque.
namespace std {
namespace chrono {
std::ostream& operator<<(std::ostream& os, system_clock::duration d) {
  return os << duration_cast<milliseconds>(d).count() << " ms";
}
}  // namespace chrono
}  // namespace std

namespace {
std::chrono::system_clock::duration TimeIt(std::function<void()> fn) {
  const auto start = std::chrono::system_clock::now();
  fn();
  return std::chrono::system_clock::now() - start;
};
}  // namespace

TEST(Speed, PushBackToFull) {
  baudvine::RingBuf<std::string, 1U << 17> standard;
  baudvine::DequeRingBuf<std::string> deque(1U << 17);

  auto standardDuration = TimeIt([&standard] {
    for (uint32_t i = 0; i < 1 << 17; i++) {
      standard.push_back("this is a moderately long string");
    }
  });

  auto dequeDuration = TimeIt([&deque] {
    for (uint32_t i = 0; i < 1 << 17; i++) {
      deque.push_back("this is a moderately long string");
    }
  });

  EXPECT_LT(standardDuration, dequeDuration);
}

TEST(Speed, PushBackOverFull) {
  baudvine::RingBuf<std::string, 2> standard;
  baudvine::DequeRingBuf<std::string> deque(2);

  auto standardDuration = TimeIt([&standard] {
    for (uint32_t i = 0; i < 1 << 17; i++) {
      standard.push_back("this is a moderately long string");
    }
  });

  auto dequeDuration = TimeIt([&deque] {
    for (uint32_t i = 0; i < 1 << 17; i++) {
      deque.push_back("this is a moderately long string");
    }
  });

  EXPECT_LT(standardDuration, dequeDuration);
}

TEST(Speed, IterateOver) {
  baudvine::RingBuf<std::string, 1U << 18> standard;
  baudvine::DequeRingBuf<std::string> deque(1U << 18);

  for (uint32_t i = 0; i < 1U << 18; i++) {
    standard.push_back("this is a moderately long string");
  }

  for (uint32_t i = 0; i < 1U << 18; i++) {
    deque.push_back("this is a moderately long string");
  }

  auto standardDuration = TimeIt([&standard] {
    for (auto& x : standard) {
      std::ignore = x;
    }
  });

  auto dequeDuration = TimeIt([&deque] {
    for (auto& x : deque) {
      std::ignore = x;
    }
  });

  EXPECT_LT(standardDuration, dequeDuration);
}
