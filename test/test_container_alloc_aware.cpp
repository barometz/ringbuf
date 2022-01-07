#include "baudvine/ringbuf/deque_ringbuf.h"
#include "baudvine/ringbuf/ringbuf.h"

#include <gtest/gtest.h>

#include <scoped_allocator>

#define EXPECT_TYPE_EQ(T1, T2) EXPECT_TRUE((std::is_same<T1, T2>::value))

template <typename RingBuf>
class ContainerReqsAllocAware : public testing::Test {};

template <typename T>
using Allocator = std::scoped_allocator_adaptor<std::allocator<T>>;

using RingBufs = testing::Types<
    baudvine::RingBuf<std::string, 2, Allocator<std::string>>,
    baudvine::DequeRingBuf<std::string, 2, Allocator<std::string>>>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerReqsAllocAware, RingBufs);

TYPED_TEST(ContainerReqsAllocAware, TypeAliases) {
  EXPECT_TYPE_EQ(Allocator<std::string>, typename TypeParam::allocator_type);
}

TYPED_TEST(ContainerReqsAllocAware, GetAllocator) {
  EXPECT_TYPE_EQ(Allocator<std::string>, decltype(TypeParam{}.get_allocator()));
}
