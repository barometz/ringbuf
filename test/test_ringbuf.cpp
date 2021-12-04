#include <gtest/gtest.h>

#include <baudvine/ringbuf/ringbuf.h>

TEST(Create, Size) {
  EXPECT_EQ((baudvine::ringbuf<int, 0>){}.size(), 0);
  EXPECT_EQ((baudvine::ringbuf<int, 1>){}.size(), 1);
  EXPECT_EQ((baudvine::ringbuf<int, 128>){}.size(), 128);
  EXPECT_EQ((baudvine::ringbuf<int, 500>){}.size(), 500);
  EXPECT_EQ((baudvine::ringbuf<int, 10 * 1024 * 1024>){}.size(),
            10 * 1024 * 1024);

  EXPECT_EQ((baudvine::ringbuf<char, 128>){}.size(), 128);
}
