#include "ringbuf_adapter.h"

#include <gtest/gtest.h>

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

  EXPECT_EQ(underTest.capacity(), 0);
  EXPECT_EQ(underTest.size(), 0);
  EXPECT_NO_THROW(underTest.push_back(53));
  EXPECT_EQ(underTest.size(), 0);
}

TEST_P(RingBuf, Capacity) {
  EXPECT_EQ((MakeAdapter<char, 128>()).capacity(), 128);
  EXPECT_EQ((MakeAdapter<int, 1>()).capacity(), 1);
  EXPECT_EQ((MakeAdapter<int, 128>()).capacity(), 128);
  EXPECT_EQ((MakeAdapter<int, 500>()).capacity(), 500);
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

class RefCounter {
 public:
  RefCounter() { counter_++; }
  RefCounter(const RefCounter&) { counter_++; }
  RefCounter(RefCounter&&) { counter_++; }
  ~RefCounter() { counter_--; }

  static int counter_;
};

int RefCounter::counter_ = 0;

TEST(RingBuf, LifeTimeStatic) {
  RefCounter::counter_ = 0;

  {
    baudvine::RingBuf<RefCounter, 3> underTest;
    EXPECT_EQ(RefCounter::counter_, 0);
  }
  EXPECT_EQ(RefCounter::counter_, 0);

  {
    baudvine::RingBuf<RefCounter, 2> underTest;
    underTest.push_back(RefCounter{});
    underTest.push_back(RefCounter{});
    EXPECT_EQ(RefCounter::counter_, 2);
    underTest.push_back(RefCounter{});
    EXPECT_EQ(RefCounter::counter_, 2);
    underTest.pop_front();
    EXPECT_EQ(RefCounter::counter_, 1);
  }
  EXPECT_EQ(RefCounter::counter_, 0);
}

TEST(RingBuf, LifeTimeDynamic) {
  RefCounter::counter_ = 0;

  {
    baudvine::DynamicRingBuf<RefCounter> underTest(3);
    EXPECT_EQ(RefCounter::counter_, 0);
  }
  EXPECT_EQ(RefCounter::counter_, 0);

  {
    baudvine::DynamicRingBuf<RefCounter> underTest(2);
    underTest.push_back(RefCounter{});
    underTest.push_back(RefCounter{});
    EXPECT_EQ(RefCounter::counter_, 2);
    underTest.push_back(RefCounter{});
    EXPECT_EQ(RefCounter::counter_, 2);
    underTest.pop_front();
    EXPECT_EQ(RefCounter::counter_, 1);
  }
  EXPECT_EQ(RefCounter::counter_, 0);
}

INSTANTIATE_TEST_SUITE_P(RingBuf,
                         RingBuf,
                         testing::Values(Variant::Static, Variant::Dynamic));
