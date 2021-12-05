#include <baudvine/ringbuf/dynamic_ringbuf.h>

#include <gtest/gtest.h>

#include <memory>

TEST(Dynamic_Zero, Zero) {
  // The degenerate case of a size zero buffer can still essentially work, it
  // just doesn't do anything useful. Consistency is king.
  baudvine::DynamicRingBuf<int> underTest(0);
  EXPECT_EQ(underTest.begin(), underTest.end());
}

TEST(Dynamic_Iterators, RangeFor) {
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
