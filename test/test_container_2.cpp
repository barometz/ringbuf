// C++20 container.requirements.general

#include "baudvine/ringbuf/ringbuf.h"
#include "instance_counter.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <limits>
#include <type_traits>
#include <vector>

#define EXPECT_TYPE_EQ(T1, T2) EXPECT_TRUE((std::is_same<T1, T2>::value))

class ContainerReqsGeneral : public testing::Test {
 public:
  void SetUp() override {
    a.clear();
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    b.clear();
    b.push_back(3);
    b.push_back(4);
    b.push_back(5);
    r.clear();
    r.push_back(5);
    r.push_back(6);
    r.push_back(7);
  }

  using X = baudvine::RingBuf<int, 2>;
  X a{};
  X b{};
  X r{};
  X&& rv = std::move(r);
};

TEST_F(ContainerReqsGeneral, TypeAliases) {
  EXPECT_TYPE_EQ(X::value_type, int);
  EXPECT_TYPE_EQ(X::reference, int&);
  EXPECT_TYPE_EQ(X::const_reference, const int&);

  using iterator = X::iterator;
  EXPECT_TYPE_EQ(iterator::value_type, int);

  using const_iterator = X::const_iterator;
  EXPECT_TRUE((std::is_convertible<iterator, const_iterator>::value));
  EXPECT_TYPE_EQ(const_iterator::value_type, int);

  using difference_type = X::difference_type;
  EXPECT_TRUE(std::is_signed<difference_type>::value);
  EXPECT_TYPE_EQ(difference_type, iterator::difference_type);
  EXPECT_TYPE_EQ(difference_type, const_iterator::difference_type);

  using size_type = X::size_type;
  EXPECT_TRUE(std::is_unsigned<size_type>::value);
  EXPECT_TRUE(std::numeric_limits<size_type>::max() >=
              std::numeric_limits<difference_type>::max());
}

TEST_F(ContainerReqsGeneral, DefaultInitialized) {
  X u;
  EXPECT_TRUE(u.empty());
}

TEST_F(ContainerReqsGeneral, ValueInitialized) {
  EXPECT_TRUE(X().empty());
}

TEST_F(ContainerReqsGeneral, DirectInitUnnamed) {
  EXPECT_THAT(X(a), testing::ContainerEq(a));
}

TEST_F(ContainerReqsGeneral, DirectInit) {
  X u(a);
  EXPECT_THAT(u, testing::ContainerEq(a));
}

TEST_F(ContainerReqsGeneral, CopyInit) {
  X u = a;
  EXPECT_THAT(u, testing::ContainerEq(a));
}

TEST_F(ContainerReqsGeneral, DirectInitMove) {
  X u(rv);
  EXPECT_THAT(u, testing::ContainerEq(r));
}

TEST_F(ContainerReqsGeneral, CopyInitMove) {
  X u = rv;
  EXPECT_THAT(u, testing::ContainerEq(r));
}

TEST_F(ContainerReqsGeneral, MoveAssignment) {
  a = rv;
  // TODO: "All existing elements of [moveAssignment] are either move assigned
  // to or destroyed", which doesn't quite match the implementation. Probably
  // deal with that together with allocator support.
  EXPECT_THAT(a, testing::ContainerEq(r));
}

TEST_F(ContainerReqsGeneral, Destructor) {
  {
    baudvine::RingBuf<InstanceCounter, 3> a;
    a.push_back({});
    a.push_back({});

    ASSERT_EQ(InstanceCounter::GetCounter(), 2);
  }
  EXPECT_EQ(InstanceCounter::GetCounter(), 0);
}

TEST_F(ContainerReqsGeneral, BeginEnd) {
  X a;  // new instance for empty case
  EXPECT_TYPE_EQ(decltype(a.begin()), X::iterator);
  EXPECT_TYPE_EQ(decltype(a.end()), X::iterator);
  EXPECT_EQ(a.begin(), a.end());

  const X constA;
  EXPECT_TYPE_EQ(decltype(constA.begin()), X::const_iterator);
  EXPECT_TYPE_EQ(decltype(constA.end()), X::const_iterator);
  EXPECT_EQ(constA.begin(), constA.end());
}

TEST_F(ContainerReqsGeneral, CbeginCend) {
  EXPECT_TYPE_EQ(decltype(a.cbegin()), X::const_iterator);
  EXPECT_TYPE_EQ(decltype(a.cend()), X::const_iterator);

  a.clear();
  EXPECT_EQ(a.cbegin(), a.cend());
}

TEST_F(ContainerReqsGeneral, Equality) {
  baudvine::RingBuf<int, 2> a;
  a.push_back(1);
  a.push_back(2);
  a.push_back(3);
  baudvine::RingBuf<int, 2> b;

  EXPECT_TRUE((std::is_convertible<decltype(a == b), bool>::value));
  EXPECT_FALSE(a == b);
  b.push_back(2);
  EXPECT_FALSE(a == b);
  b.push_back(3);
  EXPECT_TRUE(a == b);
}

TEST_F(ContainerReqsGeneral, Inequality) {
  baudvine::RingBuf<int, 2> a;
  a.push_back(1);
  a.push_back(2);
  baudvine::RingBuf<int, 2> b;

  EXPECT_TRUE((std::is_convertible<decltype(a != b), bool>::value));
  EXPECT_TRUE(a != b);
  b.push_back(1);
  b.push_back(2);
  EXPECT_FALSE(a != b);
  b.push_back(3);
  EXPECT_TRUE(a != b);
}

TEST_F(ContainerReqsGeneral, Swap) {
  EXPECT_TRUE(std::is_void<decltype(a.swap(b))>::value);

  auto ax = a;
  auto bx = b;

  a.swap(b);
  EXPECT_THAT(a, testing::Not(testing::ContainerEq(ax)));
  EXPECT_THAT(b, testing::Not(testing::ContainerEq(bx)));
  EXPECT_THAT(a, testing::ContainerEq(bx));
  EXPECT_THAT(b, testing::ContainerEq(ax));
}

TEST_F(ContainerReqsGeneral, StdSwap) {
  EXPECT_TRUE(std::is_void<decltype(std::swap(a, b))>::value);

  auto ax = a;
  auto bx = b;

  std::swap(a, b);
  EXPECT_THAT(a, testing::Not(testing::ContainerEq(ax)));
  EXPECT_THAT(b, testing::Not(testing::ContainerEq(bx)));
  EXPECT_THAT(a, testing::ContainerEq(bx));
  EXPECT_THAT(b, testing::ContainerEq(ax));
}

TEST_F(ContainerReqsGeneral, CopyAssignment) {
  EXPECT_TYPE_EQ(decltype(r = a), X&);
  r = a;
  EXPECT_THAT(r, testing::ContainerEq(a));
}

TEST_F(ContainerReqsGeneral, Size) {
  a.clear();
  EXPECT_EQ(0, std::distance(a.begin(), a.end()));
  EXPECT_EQ(0, a.size());
  a.push_back(10);
  EXPECT_EQ(1, std::distance(a.begin(), a.end()));
  EXPECT_EQ(1, a.size());
  a.push_back(20);
  EXPECT_EQ(2, std::distance(a.begin(), a.end()));
  EXPECT_EQ(2, a.size());
  a.push_back(30);
  EXPECT_EQ(2, std::distance(a.begin(), a.end()));
  EXPECT_EQ(2, a.size());
}

TEST_F(ContainerReqsGeneral, MaxSize) {
  EXPECT_EQ(a.max_size(), 2);
}

TEST_F(ContainerReqsGeneral, Empty) {
  EXPECT_FALSE(a.empty());
  a.clear();
  EXPECT_TRUE(a.empty());
}

TEST_F(ContainerReqsGeneral, IteratorComparison) {
  EXPECT_TRUE(a.begin() == a.cbegin());
  EXPECT_TRUE(a.cend() == a.end());
  EXPECT_TRUE(a.cbegin() <= a.end());
  EXPECT_TRUE(a.end() > a.cbegin());
}
