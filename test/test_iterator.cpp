// The iterator should be
// https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator

#include <baudvine/ringbuf/ringbuf.h>
#include <gtest/gtest.h>

#include <type_traits>

namespace {
baudvine::RingBuf<int, 3> GetBuf() {
  baudvine::RingBuf<int, 3> result;
  result.push_back(10);
  result.push_back(20);
  return result;
}
}  // namespace

TEST(Iterator, Deref) {
  auto buf = GetBuf();
  EXPECT_EQ(*buf.begin(), 10);
}

TEST(Iterator, Increment) {
  auto buf = GetBuf();
  auto underTest = buf.begin();
  EXPECT_EQ(*++underTest, 20);
}

TEST(Iterator, Inequality) {
  auto buf = GetBuf();
  EXPECT_NE(buf.begin(), buf.end());
}

TEST(Iterator, DerefToValueType) {
  auto buf = GetBuf();
  EXPECT_TRUE((std::is_convertible<decltype(*buf.begin()),
                                   decltype(buf.begin())::value_type>::value));
}

TEST(Iterator, ArrowDeref) {
  baudvine::RingBuf<std::string, 1> buf;
  buf.push_back("hello");
  EXPECT_EQ(buf.begin()->size(), (*buf.begin()).size());
}

TEST(Iterator, PostfixIncrement) {
  auto buf = GetBuf();
  auto i1 = buf.begin();
  auto i2 = buf.begin();
  i1++;
  ++i2;
  EXPECT_EQ(i1, i2);
}
