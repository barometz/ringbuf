#include "ringbufs.h"

#include <gtest/gtest.h>

#include <memory>
#include <scoped_allocator>

#define EXPECT_TYPE_EQ(T1, T2) EXPECT_TRUE((std::is_same<T1, T2>::value))

template <typename RingBuf>
class ContainerReqsAllocAware : public testing::Test {};

template <typename T>
using Allocator =
    std::scoped_allocator_adaptor<std::allocator<T>,
                                  std::allocator<typename T::value_type>>;

using RingBufs = AllRingBufs<std::string, 2, Allocator<std::string>>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerReqsAllocAware, RingBufs);

TYPED_TEST(ContainerReqsAllocAware, TypeAliases) {
  EXPECT_TYPE_EQ(Allocator<std::string>, typename TypeParam::allocator_type);
}

TYPED_TEST(ContainerReqsAllocAware, GetAllocator) {
  EXPECT_TYPE_EQ(Allocator<std::string>, decltype(TypeParam{}.get_allocator()));
}

TYPED_TEST(ContainerReqsAllocAware, UsesAllocator) {
  EXPECT_TRUE((std::uses_allocator<TypeParam, Allocator<std::string>>::value));
}
