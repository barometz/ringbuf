// Demonstrate that RingBuf matches
// https://en.cppreference.com/w/cpp/named_req/Container

#include "ringbuf_adapter.h"

#include <gtest/gtest.h>

class Container : public testing::TestWithParam<Variant> {
 public:
  template <typename Elem, size_t Capacity>
  RingBufAdapter<Elem, Capacity> MakeAdapter() {
    return RingBufAdapter<Elem, Capacity>(GetParam());
  }
};

TEST_P(Container, CopyCtor) {
  auto original = MakeAdapter<std::string, 2>();
  original.push_back("zero");
  original.push_back("one");
  original.push_back("two");

  RingBufAdapter<std::string, 2> copy(original);
  EXPECT_EQ(copy.at(0), "one");
  EXPECT_EQ(copy.at(1), "two");
}

TEST_P(Container, MoveCtor) {
  auto original = MakeAdapter<std::string, 2>();
  original.push_back("zero");
  original.push_back("one");
  original.push_back("two");

  RingBufAdapter<std::string, 2> moved(original);
  EXPECT_EQ(moved.at(0), "one");
  EXPECT_EQ(moved.at(1), "two");
}

TEST_P(Container, Assignment) {
  auto original = MakeAdapter<std::string, 2>();
  original.push_back("zero");
  original.push_back("one");
  original.push_back("two");

  auto copy = MakeAdapter<std::string, 2>();
  copy = original;
  EXPECT_EQ(copy.at(0), "one");
  EXPECT_EQ(copy.at(1), "two");
}

TEST_P(Container, MoveAssignment) {
  auto original = MakeAdapter<std::string, 2>();
  original.push_back("zero");
  original.push_back("one");
  original.push_back("two");

  auto copy = MakeAdapter<std::string, 2>();
  copy = std::move(original);
  EXPECT_EQ(copy.at(0), "one");
  EXPECT_EQ(copy.at(1), "two");
}

TEST_P(Container, Equality) {
  auto a = MakeAdapter<int, 3>();
  auto b = MakeAdapter<int, 3>();
  EXPECT_EQ(a, b);

  a.push_back(2010);
  a.push_back(3030);
  EXPECT_NE(a, b);

  b.push_back(2010);
  b.push_back(3030);
  EXPECT_EQ(a, b);

  b.push_back(4070);
  EXPECT_NE(a, b);
}

TEST_P(Container, Size) {
  auto underTest = MakeAdapter<float, 3>();
  EXPECT_EQ(underTest.size(), 0);

  underTest.push_back(0.4f);
  EXPECT_EQ(underTest.size(), 1);

  underTest.push_back(0.0f);
  underTest.push_back(2.4e9f);
  underTest.push_back(5.0e9f);
  EXPECT_EQ(underTest.size(), 3);
}

TEST_P(Container, MaxSize) {
  auto underTest = MakeAdapter<float, 3>();
  EXPECT_GE(underTest.max_size(), 3);
}

TEST_P(Container, Empty) {
  auto underTest = MakeAdapter<double, 2>();
  EXPECT_TRUE(underTest.empty());
  underTest.push_back(0);
  EXPECT_FALSE(underTest.empty());
  underTest.push_back(1.0);
  EXPECT_FALSE(underTest.empty());
  underTest.push_back(2.0);
  EXPECT_FALSE(underTest.empty());

  baudvine::RingBuf<double, 0> alwaysEmpty;
  EXPECT_TRUE(alwaysEmpty.empty());
  underTest.push_back(0.1);
  EXPECT_TRUE(alwaysEmpty.empty());
}

TEST_P(Container, Swap) {
  auto a = MakeAdapter<int, 3>();
  auto b = MakeAdapter<int, 3>();

  a.push_back(2010);
  a.push_back(3030);

  b.push_back(4500);
  b.push_back(20);
  b.push_back(9999);

  auto a2 = a;
  auto b2 = b;
  a2.swap(b2);

  EXPECT_EQ(a, b2);
  EXPECT_EQ(b, a2);
}

INSTANTIATE_TEST_SUITE_P(Container,
                         Container,
                         testing::Values(Variant::Standard, Variant::Deque));
