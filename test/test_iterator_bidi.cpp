// The iterator should be
// https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator

#include "ringbuf_adapter.h"
#include "ringbufs.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <string>
#include <type_traits>

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
class IteratorBidi : public testing::Test {};

using RingBufs = AllRingBufs<int, 3>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(IteratorBidi, RingBufs);

TYPED_TEST(IteratorBidi, Deref) {
  auto buf = GetBuf<TypeParam>();
  EXPECT_EQ(*buf.begin(), 10);
}

TYPED_TEST(IteratorBidi, Increment) {
  auto buf = GetBuf<TypeParam>();
  auto underTest = buf.begin();
  EXPECT_EQ(*++underTest, 20);
}

TYPED_TEST(IteratorBidi, Inequality) {
  auto buf = GetBuf<TypeParam>();
  EXPECT_NE(buf.begin(), buf.end());
}

TYPED_TEST(IteratorBidi, DerefToValueType) {
  auto buf = GetBuf<TypeParam>();
  EXPECT_TRUE(
      (std::is_convertible<decltype(*buf.begin()),
                           typename decltype(buf.begin())::value_type>::value));
}

TYPED_TEST(IteratorBidi, ArrowDeref) {
  RingBufAdapter<TypeParam, std::string, 1> buf;
  buf.push_back("hello");
  EXPECT_EQ(buf.begin()->size(), (*buf.begin()).size());
}

TYPED_TEST(IteratorBidi, PostfixIncrement) {
  auto buf = GetBuf<TypeParam>();
  auto i1 = buf.begin();
  auto i2 = buf.begin();
  i1++;
  ++i2;
  EXPECT_EQ(i1, i2);
}

// And a little extra:

TYPED_TEST(IteratorBidi, Zero) {
  // TODO not really an iterator test?
  RingBufAdapter<TypeParam, int, 0> underTest{};
  EXPECT_EQ(underTest.begin(), underTest.end());
}

TYPED_TEST(IteratorBidi, RangeFor) {
  RingBufAdapter<TypeParam, int, 4> underTest;
  underTest.push_back(41);
  underTest.push_back(40);
  underTest.push_back(39);
  underTest.push_back(38);
  underTest.push_back(37);

  auto expected = 40;
  for (auto val : underTest) {
    EXPECT_EQ(expected, val);
    expected--;
  }
}

// TODO move these out
TEST(IteratorRingBuf, Copy) {
  baudvine::RingBuf<std::string, 3> underTest;
  std::vector<std::string> copy;

  baudvine::copy(underTest.begin(), underTest.end(), std::back_inserter(copy));
  EXPECT_THAT(copy, testing::ElementsAre());

  underTest.push_back("1");
  baudvine::copy(underTest.begin(), underTest.end(), std::back_inserter(copy));
  EXPECT_THAT(copy, testing::ElementsAre("1"));

  underTest.push_back("2");
  underTest.push_back("3");
  copy.clear();
  baudvine::copy(underTest.begin(), underTest.end(), std::back_inserter(copy));
  EXPECT_THAT(copy, testing::ElementsAre("1", "2", "3"));

  underTest.push_back("4");
  underTest.push_back("5");
  copy.clear();
  baudvine::copy(underTest.begin(), underTest.end(), std::back_inserter(copy));
  EXPECT_THAT(copy, testing::ElementsAre("3", "4", "5"));

  underTest.push_back("6");
  copy.clear();
  baudvine::copy(underTest.begin(), underTest.end(), std::back_inserter(copy));
  EXPECT_THAT(copy, testing::ElementsAre("4", "5", "6"));

  underTest.pop_front();
  underTest.pop_back();
  copy.clear();
  baudvine::copy(underTest.begin(), underTest.end(), std::back_inserter(copy));
  EXPECT_THAT(copy, testing::ElementsAre("5"));

  underTest.pop_back();
  copy.clear();
  baudvine::copy(underTest.begin(), underTest.end(), std::back_inserter(copy));
  EXPECT_THAT(copy, testing::ElementsAre());
}

TEST(IteratorRingBuf, CopyAdl) {
  // Check for argument-dependent lookup of baudvine::copy. This test only uses
  // iterators from the baudvine namespace to suppress std::copy.

  baudvine::RingBuf<int, 3> a;
  baudvine::RingBuf<int, 4> b;
  std::fill_n(std::back_inserter(b), b.max_size(), 0);

  a.push_back(4);
  copy(a.begin(), a.end(), b.begin());
  EXPECT_THAT(b, testing::ElementsAre(4, 0, 0, 0));
}
