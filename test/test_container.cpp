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

TEST(Container, Equality) {
  baudvine::RingBuf<int, 3> a;
  baudvine::RingBuf<int, 3> b;
  EXPECT_EQ(a, b);

  a.Push(2010);
  a.Push(3030);
  EXPECT_NE(a, b);

  b.Push(2010);
  b.Push(3030);
  EXPECT_EQ(a, b);

  b.Push(4070);
  EXPECT_NE(a, b);
}

TEST(Container, Swap) {
  baudvine::RingBuf<int, 3> a;
  baudvine::RingBuf<int, 3> b;

  a.Push(2010);
  a.Push(3030);

  b.Push(4500);
  b.Push(20);
  b.Push(9999);

  auto a2 = a;
  auto b2 = b;
  a2.swap(b2);

  EXPECT_EQ(a, b2);
  EXPECT_EQ(b, a2);
}

TEST(Container, Size) {
  baudvine::RingBuf<float, 3> underTest;
  EXPECT_EQ(underTest.size(), 0);

  underTest.Push(0.4f);
  EXPECT_EQ(underTest.size(), 1);

  underTest.Push(0.0f);
  underTest.Push(2.4e9f);
  underTest.Push(5.0e9f);
  EXPECT_EQ(underTest.size(), 3);
}

TEST(Container, MaxSize) {
  baudvine::RingBuf<float, 3> underTest;
  // This one's a little fiddly - the actual value isn't specified since it has
  // platform and runtime limitations to deal with.
  EXPECT_GE(underTest.max_size(), std::numeric_limits<int>::max());
}

TEST(Container, Empty) {
  baudvine::RingBuf<double, 2> underTest;
  EXPECT_TRUE(underTest.empty());
  underTest.Push(0);
  EXPECT_FALSE(underTest.empty());
  underTest.Push(1.0);
  EXPECT_FALSE(underTest.empty());
  underTest.Push(2.0);
  EXPECT_FALSE(underTest.empty());

  baudvine::RingBuf<double, 0> alwaysEmpty;
  EXPECT_TRUE(alwaysEmpty.empty());
  underTest.Push(0.1);
  EXPECT_TRUE(alwaysEmpty.empty());
}
