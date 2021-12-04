#include <gtest/gtest.h>

#include <baudvine/ringbuf/ringbuf.h>

TEST(Capacity, Capacity) {
  EXPECT_EQ((baudvine::ringbuf<int, 0>){}.capacity(), 0);
  EXPECT_EQ((baudvine::ringbuf<int, 1>){}.capacity(), 1);
  EXPECT_EQ((baudvine::ringbuf<int, 128>){}.capacity(), 128);
  EXPECT_EQ((baudvine::ringbuf<int, 500>){}.capacity(), 500);
  EXPECT_EQ((baudvine::ringbuf<int, 10 * 1024 * 1024>){}.capacity(),
            10 * 1024 * 1024);

  EXPECT_EQ((baudvine::ringbuf<char, 128>){}.capacity(), 128);
}

TEST(At, Empty) {
  baudvine::ringbuf<int, 4> underTest;

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST(AtConst, Empty) {
  const baudvine::ringbuf<int, 4> underTest;

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST(Push, Push){
  baudvine::ringbuf<int, 4> underTest;
  underTest.push(56);
  EXPECT_EQ(underTest.at(0), 56);
  underTest.push(1100);
  EXPECT_EQ(underTest.at(1), 1100);
  EXPECT_EQ(underTest.at(0), 56);
  EXPECT_EQ(underTest.size(), 2);
}

TEST(Push, PushOver){
  baudvine::ringbuf<int, 2> underTest;
  underTest.push(56);
  underTest.push(1100);
  underTest.push(6500);

  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), 1100);
  EXPECT_EQ(underTest.at(1), 6500);

  underTest.push(10);
  underTest.push(12);
  underTest.push(18);
  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), 12);
  EXPECT_EQ(underTest.at(1), 18);
}
