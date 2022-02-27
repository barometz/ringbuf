#include "ringbufs.h"

#include <gmock/gmock-matchers.h>

template <typename RingBuf>
class ContainerSequence : public testing::Test {};

using RingBufs = AllRingBufs<int, 5>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerSequence, RingBufs);

TYPED_TEST(ContainerSequence, EraseSingle) {
  TypeParam underTest;
  for (size_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }

  auto it = underTest.erase(underTest.begin() + 1);
  EXPECT_THAT(underTest, testing::ElementsAre(4, 8, 10, 12));
  EXPECT_EQ(*it, 8);

  it = underTest.erase(underTest.end() - 2);
  EXPECT_THAT(underTest, testing::ElementsAre(4, 8, 12));
  EXPECT_EQ(*it, 12);

  it = underTest.erase(underTest.begin());
  EXPECT_THAT(underTest, testing::ElementsAre(8, 12));
  EXPECT_EQ(*it, 8);

  it = underTest.erase(underTest.end() - 1);
  EXPECT_THAT(underTest, testing::ElementsAre(8));
  EXPECT_EQ(it, underTest.end());

  it = underTest.erase(underTest.begin());
  EXPECT_THAT(underTest, testing::ElementsAre());
  EXPECT_EQ(it, underTest.end());
}
