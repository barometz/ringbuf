#include "ringbuf_adapter.h"

#include <gtest/gtest.h>
#include <type_traits>

// noexcept guarantees and exception safety.
template <typename T>
class ExceptionSafety : public testing::Test {};

using RingBufs =
    testing::Types<baudvine::RingBuf<int, 2>, baudvine::DequeRingBuf<int, 2>>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ExceptionSafety, RingBufs);

TYPED_TEST(ExceptionSafety, Noexcept) {
  TypeParam a;
  EXPECT_TRUE(noexcept(a.clear()));
  EXPECT_TRUE(noexcept(a.pop_back()));
  EXPECT_TRUE(noexcept(a.pop_front()));

  TypeParam b;
  EXPECT_TRUE(noexcept(a.swap(b)));
}
