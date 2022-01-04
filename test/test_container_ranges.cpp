#include "baudvine/ringbuf/deque_ringbuf.h"
#include "baudvine/ringbuf/ringbuf.h"
#include "config.h"

#include <gtest/gtest-typed-test.h>
#include <gtest/gtest.h>

#ifdef BAUDVINE_HAVE_RANGES

#include <ranges>

template <typename RingBuf>
class ContainerRanges : public testing::Test {};

TYPED_TEST_SUITE_P(ContainerRanges);

TYPED_TEST_P(ContainerRanges, IsRandomAccessRange) {
  EXPECT_TRUE(std::ranges::random_access_range<TypeParam>);
}

REGISTER_TYPED_TEST_SUITE_P(ContainerRanges, IsRandomAccessRange);

using Containers =
    ::testing::Types<baudvine::RingBuf<int, 3>, baudvine::DequeRingBuf<int, 3>>;
INSTANTIATE_TYPED_TEST_SUITE_P(My, ContainerRanges, Containers);

#endif // BAUDVINE_HAVE_RANGES
