// C++21 container.requirements.general

#include "baudvine/ringbuf/ringbuf.h"
#include "instance_counter.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <limits>
#include <type_traits>
#include <vector>

TEST(ContainerReqsGeneral, Typedefs) {
  using RingBuf = baudvine::RingBuf<int, 0>;

  EXPECT_TRUE((std::is_same<RingBuf::value_type, int>::value));
  EXPECT_TRUE((std::is_same<RingBuf::reference, int&>::value));
  EXPECT_TRUE((std::is_same<RingBuf::const_reference, const int&>::value));

  using iterator = RingBuf::iterator;
  EXPECT_TRUE((std::is_same<iterator::value_type, int>::value));

  using const_iterator = RingBuf::const_iterator;
  EXPECT_TRUE((std::is_convertible<iterator, const_iterator>::value));
  EXPECT_TRUE((std::is_same<const_iterator::value_type, int>::value));

  using difference_type = RingBuf::difference_type;
  EXPECT_TRUE(std::is_signed<difference_type>::value);
  EXPECT_TRUE(
      (std::is_same<difference_type, iterator::difference_type>::value));
  EXPECT_TRUE(
      (std::is_same<difference_type, const_iterator::difference_type>::value));

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
    baudvine::RingBuf<int, 3> copy = a;
    baudvine::RingBuf<int, 3> moveAssignment;
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
