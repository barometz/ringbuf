#include <gtest/gtest.h>

#include <baudvine/ringbuf/ringbuf.h>

TEST(Capacity, Capacity) {
  EXPECT_EQ((baudvine::RingBuf<int, 0>){}.Capacity(), 0);
  EXPECT_EQ((baudvine::RingBuf<int, 1>){}.Capacity(), 1);
  EXPECT_EQ((baudvine::RingBuf<int, 128>){}.Capacity(), 128);
  EXPECT_EQ((baudvine::RingBuf<int, 500>){}.Capacity(), 500);
  EXPECT_EQ((baudvine::RingBuf<int, 10 * 1024 * 1024>){}.Capacity(),
            10 * 1024 * 1024);

  EXPECT_EQ((baudvine::RingBuf<char, 128>){}.Capacity(), 128);
}

TEST(At, Empty) {
  baudvine::RingBuf<int, 4> underTest;

  EXPECT_THROW(underTest.At(0), std::out_of_range);
  EXPECT_THROW(underTest.At(1), std::out_of_range);
  EXPECT_THROW(underTest.At(4), std::out_of_range);
}

TEST(AtConst, Empty) {
  const baudvine::RingBuf<int, 4> underTest;

  EXPECT_THROW(underTest.At(0), std::out_of_range);
  EXPECT_THROW(underTest.At(1), std::out_of_range);
  EXPECT_THROW(underTest.At(4), std::out_of_range);
}

TEST(Push, Push) {
  baudvine::RingBuf<int, 4> underTest;
  underTest.Push(56);
  EXPECT_EQ(underTest.At(0), 56);
  underTest.Push(1100);
  EXPECT_EQ(underTest.At(1), 1100);
  EXPECT_EQ(underTest.At(0), 56);
  EXPECT_EQ(underTest.Size(), 2);
}

TEST(Push, PushOver) {
  baudvine::RingBuf<int, 2> underTest;
  underTest.Push(56);
  underTest.Push(1100);
  underTest.Push(6500);

  EXPECT_EQ(underTest.Size(), 2);
  EXPECT_EQ(underTest.At(0), 1100);
  EXPECT_EQ(underTest.At(1), 6500);

  underTest.Push(10);
  underTest.Push(12);
  underTest.Push(18);
  EXPECT_EQ(underTest.Size(), 2);
  EXPECT_EQ(underTest.At(0), 12);
  EXPECT_EQ(underTest.At(1), 18);
}

TEST(Iterators, BeginAndEnd)
{
  baudvine::RingBuf<int, 4> underTest;
  EXPECT_EQ(underTest.begin(), underTest.end());
  underTest.Push(3);
  EXPECT_EQ(std::distance(underTest.begin(), underTest.end()), 1);
  EXPECT_EQ(underTest.begin(), underTest.cbegin());
  EXPECT_EQ(underTest.end(), underTest.cend());
}

TEST(Iterators, RangeFor) {
  baudvine::RingBuf<int, 4> underTest;
  underTest.Push(41);
  underTest.Push(40);
  underTest.Push(39);
  underTest.Push(38);

  auto expected = 41;
  for (auto val : underTest) {
    EXPECT_EQ(expected, val);
    expected--;
  }
}


