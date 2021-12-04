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
  for (size_t i = 0; i < 4; i++) {
    EXPECT_NO_THROW(underTest.at(i));
  }
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST(AtConst, Empty) {
  const baudvine::ringbuf<int, 4> underTest;
  for (size_t i = 0; i < 4; i++) {
    EXPECT_NO_THROW(underTest.at(i));
  }
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

// TEST(Push, Push){
//   const baudvine::ringbuf<int, 4> underTest;
//   underTest.push(56);
//   EXPECT_EQ(underTest.at(0), 56);
//   underTest.push(1100);
//   EXPECT_EQ(underTest.at(0), 1100);
//   EXPECT_EQ(underTest.at(1), 56);
// }
