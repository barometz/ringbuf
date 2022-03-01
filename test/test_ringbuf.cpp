#include "instance_counter.h"
#include "ringbuf_adapter.h"
#include "ringbufs.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

// Functional tests that don't derive directly from the C++ container spec.
template <typename T>
class RingBuf : public testing::Test {};

using RingBufs = AllRingBufs<int, 2>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(RingBuf, RingBufs);

TYPED_TEST(RingBuf, Zero) {
  // The edge case of a size zero buffer can still essentially work, it
  // just doesn't do anything useful. Consistency is king.
  auto underTest = RingBufAdapter<TypeParam, int, 0>();

  EXPECT_EQ(underTest.max_size(), 0U);
  EXPECT_EQ(underTest.size(), 0U);
  EXPECT_NO_THROW(underTest.push_back(53));
  EXPECT_EQ(underTest.size(), 0U);
}

TYPED_TEST(RingBuf, Capacity) {
  EXPECT_EQ((RingBufAdapter<TypeParam, char, 128>()).max_size(), 128U);
  EXPECT_EQ((RingBufAdapter<TypeParam, int, 1>()).max_size(), 1U);
  EXPECT_EQ((RingBufAdapter<TypeParam, int, 128>()).max_size(), 128U);
  EXPECT_EQ((RingBufAdapter<TypeParam, int, 500>()).max_size(), 500U);
}

TYPED_TEST(RingBuf, Comparison) {
  auto a = RingBufAdapter<TypeParam, int, 3>();
  auto b = RingBufAdapter<TypeParam, int, 3>();
  auto c = RingBufAdapter<TypeParam, int, 3>();

  EXPECT_EQ(a, b);
  EXPECT_EQ(a, c);

  a.push_back(1);
  EXPECT_NE(a, b);
  b.push_back(1);
  EXPECT_EQ(a, b);
  c.push_back(2);
  EXPECT_LT(a, c);

  a.push_back(2);
  a.push_back(3);
  c.push_back(1);
  c.push_back(2);
  c.push_back(3);
  EXPECT_EQ(a, c);
}

TYPED_TEST(RingBuf, PushFrontOver) {
  auto underTest = RingBufAdapter<TypeParam, std::string, 2>();
  underTest.push_front("one");
  underTest.push_front("two");
  underTest.push_front("three");

  EXPECT_EQ(underTest.size(), 2U);
  EXPECT_EQ(underTest.at(0), "three");
  EXPECT_EQ(underTest.at(1), "two");

  underTest.push_front("five");
  underTest.push_front("six");
  underTest.push_front("seven");
  EXPECT_EQ(underTest.size(), 2U);
  EXPECT_EQ(underTest.at(0), "seven");
  EXPECT_EQ(underTest.at(1), "six");
}

TYPED_TEST(RingBuf, PopBack) {
  auto underTest = RingBufAdapter<TypeParam, int, 3>();
  underTest.push_back(41);
  underTest.pop_back();
  EXPECT_TRUE(underTest.empty());
  EXPECT_EQ(underTest.size(), 0U);

  for (auto i : {1, 2}) {
    std::ignore = i;
    underTest.push_back(42);  // push
    underTest.push_back(43);  // push
    underTest.push_back(44);  // push
    underTest.push_back(45);  // push, 42 rolls off
    underTest.pop_back();     // pop, 45 rolls off

    EXPECT_EQ(underTest.at(0), 43);
    EXPECT_EQ(underTest.size(), 2U);
  }
}

TYPED_TEST(RingBuf, DoubleEnded) {
  auto underTest = RingBufAdapter<TypeParam, int, 3>();
  underTest.push_front(1);
  underTest.push_back(2);
  underTest.push_front(3);
  underTest.push_back(4);
  underTest.push_front(5);
  underTest.pop_front();
  underTest.pop_back();
  ASSERT_EQ(underTest.size(), 1U);
  ASSERT_EQ(underTest[0], 1);
}

TYPED_TEST(RingBuf, LifeTime) {
  {
    auto underTest = RingBufAdapter<TypeParam, InstanceCounter, 3>();
    EXPECT_EQ(InstanceCounter::GetCounter(), 0);
  }
  EXPECT_EQ(InstanceCounter::GetCounter(), 0);

  {  // push/pop
    auto underTest = RingBufAdapter<TypeParam, InstanceCounter, 2>();
    underTest.push_back(InstanceCounter{});
    underTest.push_back(InstanceCounter{});
    EXPECT_EQ(InstanceCounter::GetCounter(), 2);
    underTest.push_back(InstanceCounter{});
    EXPECT_EQ(InstanceCounter::GetCounter(), 2);
    underTest.pop_front();
    EXPECT_EQ(InstanceCounter::GetCounter(), 1);
  }
  EXPECT_EQ(InstanceCounter::GetCounter(), 0);

  {  // copy
    auto underTest = RingBufAdapter<TypeParam, InstanceCounter, 2>();
    underTest.push_back(InstanceCounter{});
    underTest.push_back(InstanceCounter{});
    auto copy = underTest;
    EXPECT_EQ(InstanceCounter::GetCounter(), 4);
  }
  EXPECT_EQ(InstanceCounter::GetCounter(), 0);

  {  // move
    auto underTest = RingBufAdapter<TypeParam, InstanceCounter, 2>();
    underTest.push_back(InstanceCounter{});
    underTest.push_back(InstanceCounter{});
    auto copy = std::move(underTest);
    EXPECT_EQ(InstanceCounter::GetCounter(), 2);
  }
  EXPECT_EQ(InstanceCounter::GetCounter(), 0);
}

TYPED_TEST(RingBuf, Clear) {
  auto underTest = RingBufAdapter<TypeParam, InstanceCounter, 3>();
  EXPECT_NO_THROW(underTest.clear());
  underTest.push_back(InstanceCounter{});
  underTest.push_back(InstanceCounter{});
  EXPECT_NO_THROW(underTest.clear());
  EXPECT_EQ(underTest.size(), 0U);
}

TYPED_TEST(RingBuf, InReverse) {
  TypeParam underTest;
  underTest.push_back(1);
  underTest.push_back(2);
  underTest.push_back(3);

  std::vector<int> copy(2);
  std::copy(underTest.rbegin(), underTest.rend(), copy.begin());
  EXPECT_THAT(copy, testing::ElementsAre(3, 2));
}
