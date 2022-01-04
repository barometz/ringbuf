// Demonstrate that RingBuf matches
// https://en.cppreference.com/w/cpp/named_req/Container

#include "baudvine/ringbuf/deque_ringbuf.h"
#include "baudvine/ringbuf/ringbuf.h"
#include "config.h"

#include <gtest/gtest.h>

#ifdef BAUDVINE_HAVE_RANGES
#include <ranges>
#endif

template <typename RingBuf>
class Container : public testing::Test {};

TYPED_TEST_SUITE_P(Container);

#ifdef BAUDVINE_HAVE_RANGES
TYPED_TEST_P(Container, IsRandomAccessRange)
{
  EXPECT_TRUE(std::ranges::random_access_range<TypeParam>);
}
#endif

TYPED_TEST_P(Container, CopyCtor) {
  TypeParam original;
  original.push_back(0);
  original.push_back(1);
  original.push_back(2);
  original.push_back(3);

  TypeParam copy(original);
  EXPECT_EQ(copy.at(0), 1);
  EXPECT_EQ(copy.at(1), 2);
}

TYPED_TEST_P(Container, MoveCtor) {
  TypeParam original;
  original.push_back(0);
  original.push_back(1);
  original.push_back(2);
  original.push_back(3);

  TypeParam moved(std::move(original));
  EXPECT_EQ(moved.at(0), 1);
  EXPECT_EQ(moved.at(1), 2);
}

TYPED_TEST_P(Container, Assignment) {
  TypeParam original;
  original.push_back(0);
  original.push_back(1);
  original.push_back(2);
  original.push_back(3);

  TypeParam copy;
  copy = original;
  EXPECT_EQ(copy.at(0), 1);
  EXPECT_EQ(copy.at(1), 2);
}

TYPED_TEST_P(Container, MoveAssignment) {
  TypeParam original;
  original.push_back(0);
  original.push_back(1);
  original.push_back(2);
  original.push_back(3);

  TypeParam copy;
  copy = std::move(original);
  EXPECT_EQ(copy.at(0), 1);
  EXPECT_EQ(copy.at(1), 2);
}

TYPED_TEST_P(Container, Equality) {
  TypeParam a;
  TypeParam b;
  EXPECT_EQ(a, b);

  a.push_back(2010);
  a.push_back(3030);
  EXPECT_NE(a, b);

  b.push_back(2010);
  b.push_back(3030);
  EXPECT_EQ(a, b);

  b.push_back(4070);
  EXPECT_NE(a, b);
}

TYPED_TEST_P(Container, Size) {
  TypeParam underTest;
  EXPECT_EQ(underTest.size(), 0U);

  underTest.push_back(4);
  EXPECT_EQ(underTest.size(), 1U);

  underTest.push_back(0);
  underTest.push_back(24);
  underTest.push_back(500);
  EXPECT_EQ(underTest.size(), 3U);
}

TYPED_TEST_P(Container, MaxSize) {
  TypeParam underTest;
  EXPECT_GE(underTest.max_size(), 3U);
}

TYPED_TEST_P(Container, Empty) {
  TypeParam underTest;
  EXPECT_TRUE(underTest.empty());
  underTest.push_back(0);
  EXPECT_FALSE(underTest.empty());
  underTest.push_back(1);
  EXPECT_FALSE(underTest.empty());
  underTest.push_back(2);
  EXPECT_FALSE(underTest.empty());

  baudvine::RingBuf<double, 0> alwaysEmpty;
  EXPECT_TRUE(alwaysEmpty.empty());
  underTest.push_back(10);
  EXPECT_TRUE(alwaysEmpty.empty());
}

TYPED_TEST_P(Container, Swap) {
  TypeParam a;
  TypeParam b;

  a.push_back(2010);
  a.push_back(3030);

  b.push_back(4500);
  b.push_back(20);
  b.push_back(9999);

  auto a2 = a;
  auto b2 = b;
  a2.swap(b2);

  EXPECT_EQ(a, b2);
  EXPECT_EQ(b, a2);
}

TYPED_TEST_P(Container, BeginEnd) {
  TypeParam underTest;

  underTest.push_back(4);
  underTest.push_back(3);
  EXPECT_EQ(std::next(underTest.begin(), 2), underTest.end());

  underTest.push_back(2);
  EXPECT_EQ(std::next(underTest.begin(), 3), underTest.end());

  underTest.push_back(1);
  EXPECT_EQ(std::next(underTest.begin(), 3), underTest.end());
}

REGISTER_TYPED_TEST_SUITE_P(Container,
#ifdef BAUDVINE_HAVE_RANGES
                            IsRandomAccessRange,
#endif
                            CopyCtor,
                            MoveCtor,
                            Assignment,
                            MoveAssignment,
                            Equality,
                            Size,
                            MaxSize,
                            Empty,
                            Swap,
                            BeginEnd);
using Containers =
    ::testing::Types<baudvine::RingBuf<int, 3>, baudvine::DequeRingBuf<int, 3>>;
INSTANTIATE_TYPED_TEST_SUITE_P(My, Container, Containers);
