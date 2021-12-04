#include <gtest/gtest.h>

#include <baudvine/ringbuf/ringbuf.h>

TEST(Size, Size) {
  EXPECT_EQ((baudvine::ringbuf<int, 0>){}.size(), 0);
  EXPECT_EQ((baudvine::ringbuf<int, 1>){}.size(), 1);
  EXPECT_EQ((baudvine::ringbuf<int, 128>){}.size(), 128);
  EXPECT_EQ((baudvine::ringbuf<int, 500>){}.size(), 500);
  EXPECT_EQ((baudvine::ringbuf<int, 10 * 1024 * 1024>){}.size(),
            10 * 1024 * 1024);

  EXPECT_EQ((baudvine::ringbuf<char, 128>){}.size(), 128);
}

TEST(At, Empty) {
  baudvine::ringbuf<int, 4> underTest;
  for (size_t i = 0; i < 4; i++) {
    EXPECT_NO_THROW(underTest.at(i));
  }
}

TEST(At, OutOfBounds)
{
  baudvine::ringbuf<int, 4> underTest;
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST(AtConst, Empty) {
  const baudvine::ringbuf<int, 4> underTest;
  for (size_t i = 0; i < 4; i++) {
    EXPECT_NO_THROW(underTest.at(i));
  }
}

TEST(AtConst, OutOfBounds)
{
  const baudvine::ringbuf<int, 4> underTest;
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}
