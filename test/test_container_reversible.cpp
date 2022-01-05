#include <baudvine/ringbuf/deque_ringbuf.h>
#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>
#include <iterator>

#define EXPECT_TYPE_EQ(T1, T2) EXPECT_TRUE((std::is_same<T1, T2>::value))

template <typename RingBuf>
class ContainerReversible : public testing::Test {};

using RingBufs =
    testing::Types<baudvine::RingBuf<int, 2>, baudvine::DequeRingBuf<int, 2>>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerReversible, RingBufs);

TYPED_TEST(ContainerReversible, TypeAliases) {
  EXPECT_TYPE_EQ(typename TypeParam::reverse_iterator::value_type, int);
  EXPECT_TYPE_EQ(typename TypeParam::const_reverse_iterator::value_type, int);
  EXPECT_TYPE_EQ(typename TypeParam::reverse_iterator,
                 std::reverse_iterator<typename TypeParam::iterator>);
  EXPECT_TYPE_EQ(typename TypeParam::const_reverse_iterator,
                 std::reverse_iterator<typename TypeParam::const_iterator>);
}

TYPED_TEST(ContainerReversible, RbeginRend) {
  TypeParam a;
  EXPECT_TYPE_EQ(decltype(a.rbegin()), typename TypeParam::reverse_iterator);
  EXPECT_TYPE_EQ(decltype(a.rend()), typename TypeParam::reverse_iterator);
  EXPECT_EQ(a.rbegin(), typename TypeParam::reverse_iterator(a.end()));
  EXPECT_EQ(a.rend(), typename TypeParam::reverse_iterator(a.begin()));

  const TypeParam b;
  EXPECT_TYPE_EQ(decltype(b.rbegin()),
                 typename TypeParam::const_reverse_iterator);
  EXPECT_TYPE_EQ(decltype(b.rend()),
                 typename TypeParam::const_reverse_iterator);
  EXPECT_EQ(b.rbegin(), typename TypeParam::const_reverse_iterator(b.end()));
  EXPECT_EQ(b.rend(), typename TypeParam::const_reverse_iterator(b.begin()));
}

TYPED_TEST(ContainerReversible, CrbeginCrend) {
  TypeParam a;
  const auto& constA = a;
  EXPECT_TYPE_EQ(decltype(a.crbegin()),
                 typename TypeParam::const_reverse_iterator);
  EXPECT_TYPE_EQ(decltype(a.crend()),
                 typename TypeParam::const_reverse_iterator);
  EXPECT_EQ(a.crbegin(),
            typename TypeParam::const_reverse_iterator(constA.end()));
  EXPECT_EQ(a.crend(),
            typename TypeParam::const_reverse_iterator(constA.begin()));
}
