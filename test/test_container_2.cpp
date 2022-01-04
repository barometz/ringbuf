// C++21 container.requirements.general

#include "baudvine/ringbuf/ringbuf.h"
#include "instance_counter.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <limits>
#include <type_traits>
#include <vector>

#define EXPECT_TYPE_EQ(T1, T2) EXPECT_TRUE((std::is_same<T1, T2>::value))

TEST(ContainerReqsGeneral, Typedefs) {
  using RingBuf = baudvine::RingBuf<int, 0>;

  EXPECT_TYPE_EQ(RingBuf::value_type, int);
  EXPECT_TYPE_EQ(RingBuf::reference, int&);
  EXPECT_TYPE_EQ(RingBuf::const_reference, const int&);

  using iterator = RingBuf::iterator;
  EXPECT_TYPE_EQ(iterator::value_type, int);

  using const_iterator = RingBuf::const_iterator;
  EXPECT_TRUE((std::is_convertible<iterator, const_iterator>::value));
  EXPECT_TYPE_EQ(const_iterator::value_type, int);

  using difference_type = RingBuf::difference_type;
  EXPECT_TRUE(std::is_signed<difference_type>::value);
  EXPECT_TYPE_EQ(difference_type, iterator::difference_type);
  EXPECT_TYPE_EQ(difference_type, const_iterator::difference_type);

  using size_type = RingBuf::size_type;
  EXPECT_TRUE(std::is_unsigned<size_type>::value);
  EXPECT_TRUE(std::numeric_limits<size_type>::max() >=
              std::numeric_limits<difference_type>::max());
}

TEST(ContainerReqsGeneral, DefaultCtor) {
  baudvine::RingBuf<int, 3> defaultInitialized;
  EXPECT_TRUE(defaultInitialized.empty());

  // Value-init:
  EXPECT_TRUE((baudvine::RingBuf<int, 3>()).empty());
}

TEST(ContainerReqsGeneral, Copy) {
  baudvine::RingBuf<int, 3> a;
  a.push_back(1);
  a.push_back(2);

  EXPECT_EQ((baudvine::RingBuf<int, 3>(a)), a);

  baudvine::RingBuf<int, 3> directInit(a);
  EXPECT_EQ(directInit, a);

  baudvine::RingBuf<int, 3> copyInit = a;
  EXPECT_EQ(copyInit, a);
}

TEST(ContainerReqsGeneral, Move) {
  baudvine::RingBuf<int, 3> a;
  a.push_back(1);
  a.push_back(2);

  {
    baudvine::RingBuf<int, 3> copy = a;
    baudvine::RingBuf<int, 3> directInit(std::move(copy));
    EXPECT_EQ(directInit, a);
  }

  {
    baudvine::RingBuf<int, 3> copy = a;
    // Still called copy-initialization, oddly.
    baudvine::RingBuf<int, 3> copyInit = std::move(copy);
    EXPECT_EQ(copyInit, a);
  }

  {
    using RingBuf = baudvine::RingBuf<int, 3>;
    RingBuf copy = a;
    RingBuf moveAssignment;

    EXPECT_TYPE_EQ(decltype(moveAssignment = std::move(copy)), RingBuf&);
    moveAssignment = std::move(copy);
    // TODO: "All existing elements of [moveAssignment] are either move assigned
    // to or destroyed", which doesn't quite match the implementation. Probably
    // deal with that together with allocator support.
    EXPECT_EQ(moveAssignment, a);
  }
}

TEST(ContainerReqsGeneral, Destructor) {
  {
    baudvine::RingBuf<InstanceCounter, 3> a;
    a.push_back({});
    a.push_back({});

    ASSERT_EQ(InstanceCounter::GetCounter(), 2);
  }
  EXPECT_EQ(InstanceCounter::GetCounter(), 0);
}

TEST(ContainerReqsGeneral, BeginEnd) {
  baudvine::RingBuf<int, 3> a;
  using iterator = baudvine::RingBuf<int, 3>::iterator;
  EXPECT_TYPE_EQ(decltype(a.begin()), iterator);
  EXPECT_TYPE_EQ(decltype(a.end()), iterator);
  EXPECT_EQ(a.begin(), a.end());

  const auto constA = a;
  using const_iterator = baudvine::RingBuf<int, 3>::const_iterator;
  EXPECT_TYPE_EQ(decltype(constA.begin()), const_iterator);
  EXPECT_TYPE_EQ(decltype(constA.end()), const_iterator);
  EXPECT_EQ(constA.begin(), constA.end());
}

TEST(ContainerReqsGeneral, CbeginCend) {
  const baudvine::RingBuf<int, 3> a;
  using const_iterator = baudvine::RingBuf<int, 3>::const_iterator;
  EXPECT_TYPE_EQ(decltype(a.begin()), const_iterator);
  EXPECT_TYPE_EQ(decltype(a.end()), const_iterator);
  EXPECT_EQ(a.begin(), a.end());
}

TEST(ContainerReqsGeneral, Equality) {
  baudvine::RingBuf<int, 2> a;
  a.push_back(1);
  a.push_back(2);
  a.push_back(3);
  baudvine::RingBuf<int, 2> b;
  b.push_back(2);
  b.push_back(3);

  EXPECT_TRUE((std::is_convertible<decltype(a == b), bool>::value));
  EXPECT_TRUE(a == b);
}

TEST(ContainerReqsGeneral, InEquality) {
  baudvine::RingBuf<int, 2> a;
  a.push_back(1);
  a.push_back(2);
  baudvine::RingBuf<int, 2> b;
  b.push_back(2);
  b.push_back(3);

  EXPECT_TRUE((std::is_convertible<decltype(a != b), bool>::value));
  EXPECT_EQ(!(a == b), (a != b));
  EXPECT_TRUE(a != b);
}

TEST(ContainerReqsGeneral, Swap) {
  baudvine::RingBuf<int, 2> a;
  a.push_back(1);
  a.push_back(2);
  baudvine::RingBuf<int, 2> b;
  b.push_back(2);
  b.push_back(3);

  EXPECT_TRUE(std::is_void<decltype(a.swap(b))>::value);

  auto ax = a;
  auto bx = b;

  ax.swap(bx);
  EXPECT_EQ(ax, b);
  EXPECT_EQ(bx, a);
}

TEST(ContainerReqsGeneral, StdSwap) {
  baudvine::RingBuf<int, 2> a;
  a.push_back(1);
  a.push_back(2);
  baudvine::RingBuf<int, 2> b;
  b.push_back(2);
  b.push_back(3);

  EXPECT_TRUE(std::is_void<decltype(std::swap(a, b))>::value);

  auto ax = a;
  auto bx = b;
  
  std::swap(ax, bx);
  EXPECT_EQ(ax, b);
  EXPECT_EQ(bx, a);
}
