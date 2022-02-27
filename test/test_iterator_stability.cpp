// The iterator should be
// https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator

#include "ringbufs.h"

#include <gtest/gtest.h>

#include <iterator>

template <class RingBuf>
class IteratorStability : public testing::Test {};

using RingBufs = OurRingBufs<int, 5>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(IteratorStability, RingBufs);

TYPED_TEST(IteratorStability, PushFront) {
  // push_front() does not move end()
  TypeParam underTest;

  const auto end = underTest.end();
  for (size_t i = 0; i < underTest.max_size(); i++) {
    underTest.push_front(5);
    EXPECT_EQ(end, underTest.end());
  }
}

TYPED_TEST(IteratorStability, PushBack) {
  // push_back() does not move begin()
  TypeParam underTest;

  const auto begin = underTest.begin();
  for (size_t i = 0; i < underTest.max_size(); i++) {
    underTest.push_back(5);
    EXPECT_EQ(begin, underTest.begin());
  }
}

TYPED_TEST(IteratorStability, PopFront) {
  // pop_front does not move end().
  TypeParam underTest;
  std::fill_n(std::back_inserter(underTest), underTest.max_size() + 2, 0);

  const auto end = underTest.end();
  for (size_t i = 0; i < underTest.max_size(); i++) {
    underTest.pop_front();
    EXPECT_EQ(end, underTest.end());
  }
}

TYPED_TEST(IteratorStability, PopBack) {
  // pop_back does not move begin().
  TypeParam underTest;
  std::fill_n(std::back_inserter(underTest), underTest.max_size() + 2, 0);

  const auto begin = underTest.begin();
  for (size_t i = 0; i < underTest.max_size(); i++) {
    underTest.pop_back();
    EXPECT_EQ(begin, underTest.begin());
  }
}
