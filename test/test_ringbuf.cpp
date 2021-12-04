#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>

#include <memory>

TEST(Zero, Zero) {
  // The degenerate case of a size zero buffer can still essentially work, it
  // just doesn't do anything useful. Consistency is king.
  baudvine::RingBuf<int, 0> underTest{};

  EXPECT_EQ(underTest.begin(), underTest.end());
  EXPECT_EQ(underTest.max_size(), 0);
  EXPECT_EQ(underTest.size(), 0);
  EXPECT_NO_THROW(underTest.push_back(53));
  EXPECT_EQ(underTest.size(), 0);
}

TEST(Capacity, Capacity) {
  EXPECT_EQ((baudvine::RingBuf<char, 128>){}.max_size(), 128);
  EXPECT_EQ((baudvine::RingBuf<int, 1>){}.max_size(), 1);
  EXPECT_EQ((baudvine::RingBuf<int, 128>){}.max_size(), 128);
  EXPECT_EQ((baudvine::RingBuf<int, 500>){}.max_size(), 500);

  auto dynamic = std::make_unique<baudvine::RingBuf<int, 10 * 1024 * 1024>>();
  EXPECT_EQ(dynamic->max_size(), 10 * 1024 * 1024);
}

TEST(At, Empty) {
  baudvine::RingBuf<int, 4> underTest;

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST(AtConst, Empty) {
  const baudvine::RingBuf<int, 4> underTest{};

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST(Push, Push) {
  baudvine::RingBuf<int, 4> underTest;
  underTest.push_back(56);
  EXPECT_EQ(underTest.at(0), 56);
  underTest.push_back(1100);
  EXPECT_EQ(underTest.at(1), 1100);
  EXPECT_EQ(underTest.at(0), 56);
  EXPECT_EQ(underTest.size(), 2);
}

TEST(Push, PushOver) {
  baudvine::RingBuf<int, 2> underTest;
  underTest.push_back(56);
  underTest.push_back(1100);
  underTest.push_back(6500);

  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), 1100);
  EXPECT_EQ(underTest.at(1), 6500);

  underTest.push_back(10);
  underTest.push_back(12);
  underTest.push_back(18);
  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), 12);
  EXPECT_EQ(underTest.at(1), 18);
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

TEST(Pop, Pop) {
  baudvine::RingBuf<int, 3> underTest;
  underTest.push_back(41);
  underTest.pop_front();
  EXPECT_TRUE(underTest.empty());
  EXPECT_EQ(underTest.size(), 0);

  for (auto i : {1, 2}) {
    std::ignore = i;
    underTest.push_back(42);
    underTest.push_back(43);
    underTest.push_back(44);
    underTest.push_back(45);
    underTest.pop_front();

    EXPECT_EQ(underTest.at(0), 44);
    EXPECT_EQ(underTest.size(), 2);
  }
}
