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

TYPED_TEST(ContainerSequence, EraseRange) {
  TypeParam original;
  // 4 6 8 10 12
  for (size_t i = 0; i < original.max_size() + 2; i++) {
    original.push_back(i * 2);
  }

  // is this too many tests for one TEST()? probably.

  TypeParam underTest = original;
  auto it = underTest.erase(underTest.begin(), underTest.begin());
  EXPECT_THAT(underTest, testing::ElementsAre(4, 6, 8, 10, 12));
  EXPECT_EQ(it, underTest.begin());

  underTest = original;
  it = underTest.erase(underTest.begin(), underTest.end());
  EXPECT_THAT(underTest, testing::ElementsAre());
  EXPECT_EQ(it, underTest.end());

  underTest = original;
  it = underTest.erase(underTest.begin(), underTest.begin() + 3);
  EXPECT_THAT(underTest, testing::ElementsAre(10, 12));
  EXPECT_EQ(*it, 10);

  underTest = original;
  it = underTest.erase(underTest.end() - 3, underTest.end());
  EXPECT_THAT(underTest, testing::ElementsAre(4, 6));
  EXPECT_EQ(it, underTest.end());

  underTest = original;
  it = underTest.erase(underTest.begin() + 1, underTest.begin() + 3);
  EXPECT_THAT(underTest, testing::ElementsAre(4, 10, 12));
  EXPECT_EQ(*it, 10);

  underTest = original;
  it = underTest.erase(underTest.begin() + 2, underTest.begin() + 4);
  EXPECT_THAT(underTest, testing::ElementsAre(4, 6, 12));
  EXPECT_EQ(*it, 12);
}
