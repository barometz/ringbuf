// Demonstrate that RingBuf matches
// https://en.cppreference.com/w/cpp/named_req/Container

#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>

TEST(Container, EmptyCtor) {
  baudvine::RingBuf<char, 5>{};
}

TEST(Container, Begin) {
  baudvine::RingBuf<int, 3> underTest;
  underTest.push_back(5);
  underTest.push_back(8);
  EXPECT_EQ(*underTest.begin(), 5);
  EXPECT_EQ(*underTest.cbegin(), 5);
}

TEST(Container, End) {
  baudvine::RingBuf<int, 3> underTest;
  EXPECT_EQ(underTest.begin(), underTest.end());
  underTest.push_back(0);
  EXPECT_NE(underTest.begin(), underTest.end());
  EXPECT_EQ(std::next(underTest.begin()), underTest.end());
  EXPECT_EQ(underTest.end(), underTest.cend());
}
