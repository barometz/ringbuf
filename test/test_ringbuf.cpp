#include "ringbuf_adapter.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

class RingBuf : public testing::TestWithParam<Variant> {
 public:
  template <typename Elem, size_t Capacity>
  RingBufAdapter<Elem, Capacity> MakeAdapter() {
    return RingBufAdapter<Elem, Capacity>(GetParam());
  }
};

TEST_P(RingBuf, Zero) {
  // The edge case of a size zero buffer can still essentially work, it
  // just doesn't do anything useful. Consistency is king.
  auto underTest = MakeAdapter<int, 0>();

  EXPECT_EQ(underTest.max_size(), 0);
  EXPECT_EQ(underTest.size(), 0);
  EXPECT_NO_THROW(underTest.push_back(53));
  EXPECT_EQ(underTest.size(), 0);
}

TEST_P(RingBuf, Capacity) {
  EXPECT_EQ((MakeAdapter<char, 128>()).max_size(), 128);
  EXPECT_EQ((MakeAdapter<int, 1>()).max_size(), 1);
  EXPECT_EQ((MakeAdapter<int, 128>()).max_size(), 128);
  EXPECT_EQ((MakeAdapter<int, 500>()).max_size(), 500);
}

TEST_P(RingBuf, AtEmpty) {
  auto underTest = MakeAdapter<int, 4>();

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST_P(RingBuf, AtConstEmpty) {
  const auto underTest = MakeAdapter<int, 4>();

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST_P(RingBuf, PushBack) {
  auto underTest = MakeAdapter<std::string, 3>();
  underTest.push_back("one");
  EXPECT_EQ(underTest.at(0), "one");
  underTest.push_back("two");
  EXPECT_EQ(underTest.at(1), "two");
  EXPECT_EQ(underTest.at(0), "one");
  EXPECT_EQ(underTest.size(), 2);
}

TEST_P(RingBuf, PushOver) {
  auto underTest = MakeAdapter<std::string, 2>();
  underTest.push_back("one");
  underTest.push_back("two");
  underTest.push_back("three");

  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), "two");
  EXPECT_EQ(underTest.at(1), "three");

  underTest.push_back("five");
  underTest.push_back("six");
  underTest.push_back("seven");
  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), "six");
  EXPECT_EQ(underTest.at(1), "seven");
}

TEST_P(RingBuf, Pop) {
  auto underTest = MakeAdapter<int, 3>();
  underTest.push_back(41);
  underTest.pop_front();
  EXPECT_TRUE(underTest.empty());
  EXPECT_EQ(underTest.size(), 0);

  for (auto i : {1, 2}) {
    std::ignore = i;
    underTest.push_back(42);  // push
    underTest.push_back(43);  // push
    underTest.push_back(44);  // push
    underTest.push_back(45);  // push, 42 rolls off
    underTest.pop_front();    // pop, 43 rolls off

    EXPECT_EQ(underTest.at(0), 44);
    EXPECT_EQ(underTest.size(), 2);
  }
}

TEST_P(RingBuf, FrontBack) {
  auto underTest = MakeAdapter<int, 3>();

  underTest.push_back(4);
  underTest.push_back(3);
  EXPECT_EQ(underTest.front(), 4);
  EXPECT_EQ(underTest.back(), 3);

  underTest.push_back(2);
  underTest.push_back(1);
  EXPECT_EQ(underTest.front(), 3);
  EXPECT_EQ(underTest.back(), 1);

  underTest.pop_front();
  EXPECT_EQ(underTest.front(), 2);
  EXPECT_EQ(underTest.back(), 1);
}

TEST_P(RingBuf, Comparison) {
  auto a = MakeAdapter<int, 3>();
  auto b = MakeAdapter<int, 3>();
  auto c = MakeAdapter<int, 3>();

  EXPECT_EQ(a, b);
  EXPECT_EQ(a, c);

  a.push_back(1);
  EXPECT_NE(a, b);
  b.push_back(1);
  EXPECT_EQ(a, b);
  c.push_back(2);
  EXPECT_LT(a, c);

  a.push_back(2);
  a.push_back(3);
  c.push_back(1);
  c.push_back(2);
  c.push_back(3);
  EXPECT_EQ(a, c);
}


TEST_P(RingBuf, PushFront) {
  auto underTest = MakeAdapter<std::string, 3>();
  underTest.push_front("one");
  EXPECT_EQ(underTest.at(0), "one");
  underTest.push_front("two");
  EXPECT_EQ(underTest.at(1), "one");
  EXPECT_EQ(underTest.at(0), "two");
  EXPECT_EQ(underTest.size(), 2);
}

TEST_P(RingBuf, PushFrontOver) {
  auto underTest = MakeAdapter<std::string, 2>();
  underTest.push_front("one");
  underTest.push_front("two");
  underTest.push_front("three");

  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), "three");
  EXPECT_EQ(underTest.at(1), "two");

  underTest.push_front("five");
  underTest.push_front("six");
  underTest.push_front("seven");
  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), "seven");
  EXPECT_EQ(underTest.at(1), "six");
}

TEST_P(RingBuf, PopBack) {
  auto underTest = MakeAdapter<int, 3>();
  underTest.push_back(41);
  underTest.pop_back();
  EXPECT_TRUE(underTest.empty());
  EXPECT_EQ(underTest.size(), 0);

  for (auto i : {1, 2}) {
    std::ignore = i;
    underTest.push_back(42);  // push
    underTest.push_back(43);  // push
    underTest.push_back(44);  // push
    underTest.push_back(45);  // push, 42 rolls off
    underTest.pop_back();    // pop, 45 rolls off

    EXPECT_EQ(underTest.at(0), 43);
    EXPECT_EQ(underTest.size(), 2);
  }
}

TEST_P(RingBuf, DoubleEnded) {
  auto underTest = MakeAdapter<int, 3>();
  underTest.push_front(1);
  underTest.push_back(2);
  underTest.push_front(3);
  underTest.push_back(4);
  underTest.push_front(5);
  underTest.pop_front();
  underTest.pop_back();
  ASSERT_EQ(underTest.size(), 1);
  ASSERT_EQ(underTest[0], 1);
}

class RefCounter {
 public:
  RefCounter() { counter_++; }
  RefCounter(const RefCounter& /*other*/) { counter_++; }
  RefCounter(RefCounter&& /*other*/) noexcept { counter_++; }
  ~RefCounter() { counter_--; }

  static int counter_;
};

int RefCounter::counter_ = 0;

TEST_P(RingBuf, LifeTime) {
  {
    auto underTest = MakeAdapter<RefCounter, 3>();
    EXPECT_EQ(RefCounter::counter_, 0);
  }
  EXPECT_EQ(RefCounter::counter_, 0);

  {  // push/pop
    auto underTest = MakeAdapter<RefCounter, 2>();
    underTest.push_back(RefCounter{});
    underTest.push_back(RefCounter{});
    EXPECT_EQ(RefCounter::counter_, 2);
    underTest.push_back(RefCounter{});
    EXPECT_EQ(RefCounter::counter_, 2);
    underTest.pop_front();
    EXPECT_EQ(RefCounter::counter_, 1);
  }
  EXPECT_EQ(RefCounter::counter_, 0);

  {  // copy
    auto underTest = MakeAdapter<RefCounter, 2>();
    underTest.push_back(RefCounter{});
    underTest.push_back(RefCounter{});
    auto copy = underTest;
    EXPECT_EQ(RefCounter::counter_, 4);
  }
  EXPECT_EQ(RefCounter::counter_, 0);

  {  // move
    auto underTest = MakeAdapter<RefCounter, 2>();
    underTest.push_back(RefCounter{});
    underTest.push_back(RefCounter{});
    auto copy = std::move(underTest);
    EXPECT_EQ(RefCounter::counter_, 2);
  }
  EXPECT_EQ(RefCounter::counter_, 0);
}

TEST_P(RingBuf, Clear) {
  auto underTest = MakeAdapter<RefCounter, 3>();
  EXPECT_NO_THROW(underTest.clear());
  underTest.push_back(RefCounter{});
  underTest.push_back(RefCounter{});
  EXPECT_NO_THROW(underTest.clear());
  EXPECT_EQ(underTest.size(), 0);
}

INSTANTIATE_TEST_SUITE_P(RingBuf,
                         RingBuf,
                         testing::Values(Variant::Standard, Variant::Deque));
