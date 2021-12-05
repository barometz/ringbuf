// The iterator should be
// https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator

#include <baudvine/ringbuf/dynamic_ringbuf.h>
#include <gtest/gtest.h>

#include <type_traits>

namespace {
baudvine::DynamicRingBuf<int> GetBuf() {
  baudvine::DynamicRingBuf<int> result(3);
  result.push_back(10);
  result.push_back(20);
  return result;
}
}  // namespace

TEST(IteratorDyn, Deref) {
  auto buf = GetBuf();
  EXPECT_EQ(*buf.begin(), 10);
}

TEST(IteratorDyn, Increment) {
  auto buf = GetBuf();
  auto underTest = buf.begin();
  EXPECT_EQ(*++underTest, 20);
}

TEST(IteratorDyn, Inequality) {
  auto buf = GetBuf();
  EXPECT_NE(buf.begin(), buf.end());
}

TEST(IteratorDyn, DerefToValueType) {
  auto buf = GetBuf();
  EXPECT_TRUE((std::is_convertible<decltype(*buf.begin()),
                                   decltype(buf.begin())::value_type>::value));
}

TEST(IteratorDyn, ArrowDeref) {
  baudvine::DynamicRingBuf<std::string> buf(1);
  buf.push_back("hello");
  EXPECT_EQ(buf.begin()->size(), (*buf.begin()).size());
}

TEST(IteratorDyn, PostfixIncrement) {
  auto buf = GetBuf();
  auto i1 = buf.begin();
  auto i2 = buf.begin();
  i1++;
  ++i2;
  EXPECT_EQ(i1, i2);
}

// And a little extra:

TEST(IteratorDyn, Zero) {
  baudvine::DynamicRingBuf<int> underTest(0);
  EXPECT_EQ(underTest.begin(), underTest.end());
}

TEST(IteratorDyn, RangeFor) {
  baudvine::DynamicRingBuf<int> underTest(4);
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

