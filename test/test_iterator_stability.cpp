// The iterator should be
// https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator

#include "ringbufs.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <iterator>

namespace {
template <typename T>
T GetBuf() {
  T result;
  result.push_back(10);
  result.push_back(20);
  return result;
}
}  // namespace

template <class RingBuf>
class IteratorStability : public testing::Test {};

using RingBufs = AllRingBufs<int, 5>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(IteratorStability, RingBufs);

TYPED_TEST(IteratorStability, PushBack) {
  // When an element is pushed back, begin() should only change when we were full.
  TypeParam underTest;
  const auto begin = underTest.begin();
  
  std::fill_n(std::back_inserter(underTest), 4, 0);
  EXPECT_EQ(begin, underTest.begin());
  
  underTest.push_back(0);
  EXPECT_EQ(begin, underTest.begin());

  underTest.push_back(0);
  EXPECT_NE(begin, underTest.begin());
}

TYPED_TEST(IteratorStability, PopFront) {
  // When an element is pushed back, begin() should only change when we were full.
  TypeParam underTest;
  
  std::fill_n(std::back_inserter(underTest), underTest.max_size() + 2, 0);
  const auto end = underTest.end();
  underTest.pop_front();
  EXPECT_EQ(end, underTest.end());

  underTest.pop_front();
  underTest.pop_front();
  underTest.pop_front();
  underTest.pop_front();
  EXPECT_EQ(end, underTest.end());
}
