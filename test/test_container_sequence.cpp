#include "instance_counter.h"
#include "ringbuf_adapter.h"
#include "ringbufs.h"

#include <gmock/gmock-matchers.h>

// Tests for the Sequence Container requirements.
// https://timsong-cpp.github.io/cppwp/n4868/sequence.reqmts#4 and #14
template <typename RingBuf>
class ContainerSequence : public testing::Test {};

using RingBufs = AllRingBufs<int, 5>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerSequence, RingBufs);

TYPED_TEST(ContainerSequence, EraseSingle) {
  TypeParam underTest;
  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
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
  for (uint16_t i = 0; i < original.max_size() + 2; i++) {
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

TYPED_TEST(ContainerSequence, EraseSmallRangeInLowerMiddle) {
  RingBufAdapter<TypeParam, int, 7> underTest;
  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }
  // the important bit here is that the range is in the bottom half, and shorter
  // than the leading elements.
  auto it = underTest.erase(underTest.begin() + 2, underTest.begin() + 3);
  EXPECT_THAT(underTest, testing::ElementsAre(4, 6, 10, 12, 14, 16));
  EXPECT_EQ(*it, 10);
}

TYPED_TEST(ContainerSequence, EraseSmallRangeInUpperMiddle) {
  RingBufAdapter<TypeParam, int, 7> underTest;
  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }
  // the important bit here is that the range is in the upper half, and shorter
  // than the trailing elements.
  auto it = underTest.erase(underTest.end() - 3, underTest.end() - 2);
  EXPECT_THAT(underTest, testing::ElementsAre(4, 6, 8, 10, 14, 16));
  EXPECT_EQ(*it, 14);
}

TYPED_TEST(ContainerSequence, Front) {
  TypeParam underTest;
  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }

  EXPECT_EQ(underTest.front(), 4);
  underTest.pop_front();
  EXPECT_EQ(underTest.front(), 6);

  underTest.front() = 9;
  const auto constUnderTest = underTest;
  EXPECT_EQ(constUnderTest.front(), 9);
}

TYPED_TEST(ContainerSequence, Back) {
  TypeParam underTest;
  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }

  EXPECT_EQ(underTest.back(), 12);
  underTest.pop_back();
  EXPECT_EQ(underTest.back(), 10);

  underTest.back() = 9;
  const auto constUnderTest = underTest;
  EXPECT_EQ(constUnderTest.back(), 9);
}

TYPED_TEST(ContainerSequence, EmplaceFront) {
  RingBufAdapter<TypeParam, std::string, 2> underTest;
  EXPECT_EQ(underTest.emplace_front(5, 'a'), "aaaaa");
  EXPECT_EQ(underTest.emplace_front(5, 'b'), "bbbbb");
  EXPECT_EQ(underTest.emplace_front(5, 'c'), "ccccc");
  EXPECT_THAT(underTest, testing::ElementsAre("ccccc", "bbbbb"));

  underTest.emplace_front(5, 'd') = "haha, nope!";
  EXPECT_EQ(underTest.front(), "haha, nope!");
}

TYPED_TEST(ContainerSequence, EmplaceBack) {
  RingBufAdapter<TypeParam, std::string, 2> underTest;
  EXPECT_EQ(underTest.emplace_back(5, 'a'), "aaaaa");
  EXPECT_EQ(underTest.emplace_back(5, 'b'), "bbbbb");
  EXPECT_EQ(underTest.emplace_back(5, 'c'), "ccccc");
  EXPECT_THAT(underTest, testing::ElementsAre("bbbbb", "ccccc"));

  underTest.emplace_back(5, 'd') = "haha, nope!";
  EXPECT_EQ(underTest.back(), "haha, nope!");
}

TYPED_TEST(ContainerSequence, PushFront) {
  RingBufAdapter<TypeParam, InstanceCounter, 2> underTest;
  InstanceCounter source;
  underTest.push_front(source);
  underTest.push_front(std::move(source));
  EXPECT_TRUE(underTest[0].IsMoved());
  EXPECT_TRUE(underTest[1].IsCopy());
}

TYPED_TEST(ContainerSequence, PushBack) {
  RingBufAdapter<TypeParam, InstanceCounter, 2> underTest;
  InstanceCounter source;
  underTest.push_back(source);
  underTest.push_back(std::move(source));
  EXPECT_TRUE(underTest[0].IsCopy());
  EXPECT_TRUE(underTest[1].IsMoved());
}

TYPED_TEST(ContainerSequence, PopFront) {
  TypeParam underTest;
  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }
  underTest.pop_front();
  EXPECT_EQ(underTest.front(), 6);
}

TYPED_TEST(ContainerSequence, PopBack) {
  TypeParam underTest;
  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }
  underTest.pop_back();
  EXPECT_EQ(underTest.back(), 10);
}

TYPED_TEST(ContainerSequence, Subscript) {
  TypeParam underTest;
  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }
  EXPECT_EQ(underTest[0], 4);
  EXPECT_EQ(underTest[4], 12);

  underTest[1] = 555;
  const auto constUnderTest = underTest;
  EXPECT_EQ(underTest[1], 555);
  EXPECT_NO_THROW(underTest[5]);
}

TYPED_TEST(ContainerSequence, At) {
  TypeParam underTest;

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);

  for (uint16_t i = 0; i < underTest.max_size() + 2; i++) {
    underTest.push_back(i * 2);
  }
  EXPECT_EQ(underTest.at(0), 4);
  EXPECT_EQ(underTest.at(4), 12);

  underTest.at(1) = 555;
  const auto constUnderTest = underTest;
  EXPECT_EQ(underTest.at(1), 555);

  EXPECT_THROW(underTest.at(5), std::out_of_range);
  EXPECT_THROW(underTest.at(static_cast<typename TypeParam::size_type>(-1)),
               std::out_of_range);
}

TYPED_TEST(ContainerSequence, AtConstEmpty) {
  const TypeParam underTest;

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}
