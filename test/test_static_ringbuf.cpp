#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>

#include <memory>

TEST(Zero, Zero) {
  baudvine::RingBuf<int, 0> underTest{};
  EXPECT_EQ(underTest.begin(), underTest.end());
}

TEST(Iterators, RangeFor) {
  baudvine::RingBuf<int, 4> underTest;
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
