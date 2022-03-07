#include "baudvine/deque_ringbuf.h"
#include "baudvine/ringbuf.h"

#include "unflex_ringbuf.h"

#include <gtest/gtest.h>

#include <chrono>

// TODO: use black-box method to prevent inconvenient compiler optimizations

// Speed comparison between deque and standard. Standard should generally be at
// least as fast as deque, but in practice we're not the only process so there's
// going to be some noise.
namespace std {
namespace chrono {
std::ostream& operator<<(std::ostream& os, system_clock::duration d) {
  return os << duration_cast<microseconds>(d).count() << " µs";
}
}  // namespace chrono
}  // namespace std

namespace {

constexpr uint64_t kTestSize = 1 << 25;

std::chrono::system_clock::duration TimeIt(const std::function<void()>& fn) {
  const auto start = std::chrono::system_clock::now();
  fn();
  return std::chrono::system_clock::now() - start;
}
}  // namespace

TEST(Speed, PushBackToFull) {
  baudvine::RingBuf<uint64_t, kTestSize> standard;
  UnFlexRingBuf<uint64_t, kTestSize> flex;
  baudvine::DequeRingBuf<uint64_t, kTestSize> deque;

  // Preload everything once so all the memory is definitely allocated.
  for (uint32_t i = 0; i < standard.capacity(); i++) {
    standard.push_back(0);
  }
  standard.clear();
  for (uint32_t i = 0; i < flex.capacity(); i++) {
    flex.push_back(0);
  }
  flex.clear();
  for (uint32_t i = 0; i < deque.capacity(); i++) {
    deque.push_back(0);
  }
  deque.clear();

  auto standardDuration = TimeIt([&standard] {
    for (uint32_t i = 0; i < standard.capacity(); i++) {
      standard.push_back(0);
    }
  });

  auto flexDuration = TimeIt([&flex] {
    for (uint32_t i = 0; i < flex.capacity(); i++) {
      flex.push_back(0);
    }
  });

  auto dequeDuration = TimeIt([&deque] {
    for (uint32_t i = 0; i < deque.capacity(); i++) {
      deque.push_back(0);
    }
  });

  EXPECT_LT(standardDuration, dequeDuration);
  std::cout << "RingBuf:      " << standardDuration << std::endl;
  std::cout << "FlexRingBuf:  " << flexDuration << std::endl;
  std::cout << "DequeRingBuf: " << dequeDuration << std::endl;
}

TEST(Speed, PushBackOverFull) {
  baudvine::RingBuf<uint64_t, 3> standard;
  UnFlexRingBuf<uint64_t, 3> flex;
  baudvine::DequeRingBuf<uint64_t, 3> deque;

  auto standardDuration = TimeIt([&standard] {
    for (uint32_t i = 0; i < kTestSize; i++) {
      standard.push_back(0);
    }
  });

  auto flexDuration = TimeIt([&flex] {
    for (uint32_t i = 0; i < kTestSize; i++) {
      flex.push_back(0);
    }
  });

  auto dequeDuration = TimeIt([&deque] {
    for (uint32_t i = 0; i < kTestSize; i++) {
      deque.push_back(0);
    }
  });

  EXPECT_LT(standardDuration, dequeDuration);
  std::cout << "RingBuf:      " << standardDuration << std::endl;
  std::cout << "FlexRingBuf:  " << flexDuration << std::endl;
  std::cout << "DequeRingBuf: " << dequeDuration << std::endl;
}

TEST(Speed, IterateOver) {
  baudvine::RingBuf<uint64_t, kTestSize> standard;
  UnFlexRingBuf<uint64_t, kTestSize> flex;
  baudvine::DequeRingBuf<uint64_t, kTestSize> deque;

  for (uint32_t i = 0; i < standard.capacity(); i++) {
    standard.push_back(i);
  }
  for (uint32_t i = 0; i < flex.capacity(); i++) {
    flex.push_back(i);
  }
  for (uint32_t i = 0; i < deque.capacity(); i++) {
    deque.push_back(i);
  }

  // Do a little math so the compiler doesn't optimize the test away in a
  // release build.

  uint64_t acc{};
  auto flexDuration = TimeIt([&flex, &acc] {
    for (auto& x : flex) {
      acc += x;
    }
  });

  auto standardDuration = TimeIt([&standard, &acc] {
    for (auto& x : standard) {
      acc += x;
    }
  });

  auto dequeDuration = TimeIt([&deque, &acc] {
    for (auto& x : deque) {
      acc += x;
    }
  });

  EXPECT_LT(standardDuration, dequeDuration);
  std::cout << "RingBuf:      " << standardDuration << std::endl;
  std::cout << "FlexRingBuf:  " << flexDuration << std::endl;
  std::cout << "DequeRingBuf: " << dequeDuration << std::endl;
}

TEST(Speed, Copy) {
  // baudvine::copy should be faster than std::copy as std::copy doesn't know
  // that there are at most two contiguous sections.
  baudvine::RingBuf<int, kTestSize> fixed;
  std::fill_n(std::back_inserter(fixed), kTestSize, 55);
  baudvine::FlexRingBuf<int> flex(kTestSize);
  std::fill_n(std::back_inserter(flex), kTestSize, 55);

  std::vector<int> copy;
  copy.reserve(kTestSize);
  std::fill_n(std::back_inserter(copy), kTestSize, 44);

  auto customTime = TimeIt([&fixed, &copy] {
    baudvine::copy(fixed.begin(), fixed.end(), copy.begin());
  });

  auto customTimeFlex = TimeIt([&flex, &copy] {
    baudvine::copy(flex.begin(), flex.end(), copy.begin());
  });

  auto standardTime = TimeIt(
      [&fixed, &copy] { std::copy(fixed.begin(), fixed.end(), copy.begin()); });

  EXPECT_LT(customTime, standardTime);
  std::cout << "baudvine::copy (fixed): " << customTime << std::endl;
  std::cout << "baudvine::copy (flex):  " << customTimeFlex << std::endl;
  std::cout << "std::copy:              " << standardTime << std::endl;
}
