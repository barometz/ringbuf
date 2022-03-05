#include "ringbufs.h"

#include <gtest/gtest.h>

// Functional tests that don't derive directly from the C++ container spec.
template <typename T>
class Iterator : public testing::Test {};

using RingBufs = OurRingBufs<int, 2>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(Iterator, RingBufs);

#ifndef NDEBUG
TYPED_TEST(Iterator, OutOfBounds) {
  TypeParam buffer;
  buffer.emplace_back();
  buffer.emplace_back();
  EXPECT_THROW(*(buffer.begin() - 1), std::runtime_error);
  EXPECT_THROW(*(buffer.end() + 1), std::runtime_error);
}
#endif // NDEBUG
