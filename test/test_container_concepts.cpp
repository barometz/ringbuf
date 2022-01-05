#include "baudvine/ringbuf/deque_ringbuf.h"
#include "baudvine/ringbuf/ringbuf.h"
#include "config.h"

#include <gtest/gtest-typed-test.h>
#include <gtest/gtest.h>

#ifdef BAUDVINE_HAVE_RANGES

#include <ranges>

template <typename RingBuf>
class ContainerConcepts : public testing::Test {};

using RingBufs =
    testing::Types<baudvine::RingBuf<int, 2>, baudvine::DequeRingBuf<int, 2>>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerConcepts, RingBufs);

TYPED_TEST(ContainerConcepts, IsBidiRange) {
  EXPECT_TRUE(std::ranges::bidirectional_range<TypeParam>);
}

#endif  // BAUDVINE_HAVE_RANGES
