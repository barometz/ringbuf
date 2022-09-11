#include "baudvine/deque_ringbuf.h"
#include "baudvine/ringbuf.h"

#include "black_box.h"

#include <gtest/gtest.h>

#include <chrono>

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
  baudvine::DequeRingBuf<uint64_t, kTestSize> deque;

  // Preload everything once so all the memory is definitely allocated
  for (uint32_t i = 0; i < standard.max_size(); i++) {
    standard.push_back(black_box(i));
  }
  standard.clear();
  for (uint32_t i = 0; i < deque.max_size(); i++) {
    deque.push_back(black_box(i));
  }
  deque.clear();

  auto standardDuration = TimeIt([&standard] {
    for (uint32_t i = 0; i < standard.max_size(); i++) {
      standard.push_back(black_box(i));
    }
    do_nothing(&standard);
  });

  auto dequeDuration = TimeIt([&deque] {
    for (uint32_t i = 0; i < deque.max_size(); i++) {
      deque.push_back(black_box(i));
    }
    do_nothing(&deque);
  });

  EXPECT_LT(standardDuration, dequeDuration);
  std::cout << "RingBuf:      " << standardDuration << std::endl;
  std::cout << "DequeRingBuf: " << dequeDuration << std::endl;
}

TEST(Speed, PushBackOverFull) {
  baudvine::RingBuf<uint64_t, 3> standard;
  baudvine::DequeRingBuf<uint64_t, 3> deque;

  auto standardDuration = TimeIt([&standard] {
    for (uint32_t i = 0; i < kTestSize; i++) {
      standard.push_back(black_box(i));
    }
    do_nothing(&standard);
  });

  auto dequeDuration = TimeIt([&deque] {
    for (uint32_t i = 0; i < kTestSize; i++) {
      deque.push_back(black_box(i));
    }
    do_nothing(&deque);
  });

  EXPECT_LT(standardDuration, dequeDuration);
  std::cout << "RingBuf:      " << standardDuration << std::endl;
  std::cout << "DequeRingBuf: " << dequeDuration << std::endl;
}

TEST(Speed, IterateOver) {
  baudvine::RingBuf<uint64_t, kTestSize> standard;
  baudvine::DequeRingBuf<uint64_t, kTestSize> deque;

  for (uint32_t i = 0; i < standard.max_size(); i++) {
    standard.push_back(black_box(i));
  }

  for (uint32_t i = 0; i < deque.max_size(); i++) {
    deque.push_back(black_box(i));
  }

  auto standardDuration = TimeIt([&standard] {
    for (auto& x : standard) {
      black_box(x);
    }
  });

  auto dequeDuration = TimeIt([&deque] {
    for (auto& x : deque) {
      black_box(x);
    }
  });

  EXPECT_LT(standardDuration, dequeDuration);
  std::cout << "RingBuf:      " << standardDuration << std::endl;
  std::cout << "DequeRingBuf: " << dequeDuration << std::endl;
}

TEST(Speed, Copy) {
  // baudvine::copy should be faster than std::copy as std::copy doesn't know
  // that there are at most two contiguous sections.
  baudvine::RingBuf<int, kTestSize> underTest;
  std::fill_n(std::back_inserter(underTest), kTestSize, 55);

  std::vector<int> copy;
  copy.reserve(kTestSize);
  std::fill_n(std::back_inserter(copy), kTestSize, 44);

  auto customTime = TimeIt([&underTest, &copy] {
    baudvine::copy(underTest.begin(), underTest.end(), copy.begin());
    do_nothing(&copy);
  });

  auto standardTime = TimeIt([&underTest, &copy] {
    std::copy(underTest.begin(), underTest.end(), copy.begin());
    do_nothing(&copy);
  });

  EXPECT_LT(customTime, standardTime);
  std::cout << "baudvine::copy: " << customTime << std::endl;
  std::cout << "std::copy:      " << standardTime << std::endl;
}
