// Demonstrate that RingBuf matches
// https://en.cppreference.com/w/cpp/named_req/Container

#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>

TEST(Container, EmptyCtor) {
  baudvine::RingBuf<char, 5>{};
}

TEST(Container, CopyCtor) {
  baudvine::RingBuf<std::string, 2> original;
  original.Push("zero");
  original.Push("one");
  original.Push("two");

  baudvine::RingBuf<std::string, 2> copy(original);
  EXPECT_EQ(copy.At(0), "one");
  EXPECT_EQ(copy.At(1), "two");
}

TEST(Container, MoveCtor) {
  baudvine::RingBuf<std::string, 2> original;
  original.Push("zero");
  original.Push("one");
  original.Push("two");

  baudvine::RingBuf<std::string, 2> moved(std::move(original));
  EXPECT_EQ(moved.At(0), "one");
  EXPECT_EQ(moved.At(1), "two");
}

TEST(Container, Assignment) {
  baudvine::RingBuf<std::string, 2> original;
  original.Push("zero");
  original.Push("one");
  original.Push("two");

  baudvine::RingBuf<std::string, 2> copy{};
  copy = original;
  EXPECT_EQ(copy.At(0), "one");
  EXPECT_EQ(copy.At(1), "two");
}

TEST(Container, MoveAssignment) {
  baudvine::RingBuf<std::string, 2> original;
  original.Push("zero");
  original.Push("one");
  original.Push("two");

  baudvine::RingBuf<std::string, 2> copy{};
  copy = std::move(original);
  EXPECT_EQ(copy.At(0), "one");
  EXPECT_EQ(copy.At(1), "two");
}

TEST(Container, Begin) {
  baudvine::RingBuf<int, 3> underTest;
  underTest.Push(5);
  underTest.Push(8);
  EXPECT_EQ(*underTest.begin(), 5);
  EXPECT_EQ(*underTest.cbegin(), 5);
}

TEST(Container, End) {
  baudvine::RingBuf<int, 3> underTest;
  EXPECT_EQ(underTest.begin(), underTest.end());
  underTest.Push(0);
  EXPECT_NE(underTest.begin(), underTest.end());
  EXPECT_EQ(std::next(underTest.begin()), underTest.end());
  EXPECT_EQ(underTest.end(), underTest.cend());
}
