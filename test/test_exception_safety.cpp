#include "ringbuf_adapter.h"
#include "ringbufs.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <stdexcept>
#include <type_traits>

// noexcept guarantees and exception safety.
template <typename T>
class ExceptionSafety : public testing::Test {};

using RingBufs = AllRingBufs<int, 2>;
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

class Thrower {
 public:
  Thrower(int value, bool should_throw) : value_(value) {
    if (should_throw) {
      throw std::runtime_error("error");
    }
  }

  explicit Thrower(int value) : Thrower(value, false){};

  ~Thrower() { value_ = 0xDEADBEEF; }

  friend bool operator==(const Thrower& lhs, const Thrower& rhs) {
    return lhs.value_ == rhs.value_;
  }

 private:
  int value_;
};

TYPED_TEST(ExceptionSafety, EmplaceBack) {
  RingBufAdapter<TypeParam, Thrower, 2> underTest;

  EXPECT_THROW(underTest.emplace_back(0, true), std::runtime_error);
  EXPECT_TRUE(underTest.empty());

  underTest.push_back(Thrower(1));
  underTest.push_back(Thrower(2));
  EXPECT_THROW(underTest.emplace_back(3, true), std::runtime_error);
  EXPECT_THAT(underTest, testing::ElementsAre(Thrower(1), Thrower(2)));

  underTest.push_back(Thrower(4));
  EXPECT_THROW(underTest.emplace_back(5, true), std::runtime_error);
  EXPECT_THAT(underTest, testing::ElementsAre(Thrower(2), Thrower(4)));
}

TYPED_TEST(ExceptionSafety, EmplaceFront) {
  RingBufAdapter<TypeParam, Thrower, 2> underTest;

  EXPECT_THROW(underTest.emplace_front(0, true), std::runtime_error);
  EXPECT_TRUE(underTest.empty());

  underTest.push_front(Thrower(1));
  underTest.push_front(Thrower(2));
  EXPECT_THROW(underTest.emplace_front(3, true), std::runtime_error);
  EXPECT_THAT(underTest, testing::ElementsAre(Thrower(2), Thrower(1)));

  underTest.push_front(Thrower(4));
  EXPECT_THROW(underTest.emplace_front(5, true), std::runtime_error);
  EXPECT_THAT(underTest, testing::ElementsAre(Thrower(4), Thrower(2)));
}
