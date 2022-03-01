#include "baudvine/ringbuf/deque_ringbuf.h"
#include "baudvine/ringbuf/ringbuf.h"
#include "instance_counter.h"
#include "ringbuf_adapter.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <limits>
#include <type_traits>
#include <vector>

#define EXPECT_TYPE_EQ(T1, T2) EXPECT_TRUE((std::is_same<T1, T2>::value))

// C++20 container.requirements.general
template <typename RingBuf>
class ContainerReqsGeneral : public testing::Test {
 public:
  void SetUp() override {
    a_.clear();
    a_.push_back(1);
    a_.push_back(2);
    a_.push_back(3);
    b_.clear();
    b_.push_back(3);
    b_.push_back(4);
    b_.push_back(5);
    r_.clear();
    r_.push_back(5);
    r_.push_back(6);
    r_.push_back(7);
  }

  RingBuf a_{};
  RingBuf b_{};
  RingBuf r_{};
  RingBuf&& rv_ = std::move(r_);
};

using RingBufs =
    testing::Types<baudvine::RingBuf<int, 2>, baudvine::DequeRingBuf<int, 2>>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerReqsGeneral, RingBufs);

TYPED_TEST(ContainerReqsGeneral, TypeAliases) {
  EXPECT_TYPE_EQ(typename TypeParam::value_type, int);
  EXPECT_TYPE_EQ(typename TypeParam::reference, int&);
  EXPECT_TYPE_EQ(typename TypeParam::const_reference, const int&);

  using iterator = typename TypeParam::iterator;
  EXPECT_TYPE_EQ(typename iterator::value_type, int);

  using const_iterator = typename TypeParam::const_iterator;
  EXPECT_TRUE((std::is_convertible<iterator, const_iterator>::value));
  EXPECT_TYPE_EQ(typename const_iterator::value_type, int);

  using difference_type = typename TypeParam::difference_type;
  EXPECT_TRUE(std::is_signed<difference_type>::value);
  EXPECT_TYPE_EQ(difference_type, typename iterator::difference_type);
  EXPECT_TYPE_EQ(difference_type, typename const_iterator::difference_type);

  using size_type = typename TypeParam::size_type;
  EXPECT_TRUE(std::is_unsigned<size_type>::value);
  EXPECT_TRUE(std::numeric_limits<size_type>::max() >=
              std::numeric_limits<difference_type>::max());
}

TYPED_TEST(ContainerReqsGeneral, DefaultInitialized) {
  TypeParam u;
  EXPECT_TRUE(u.empty());
}

TYPED_TEST(ContainerReqsGeneral, ValueInitialized) {
  EXPECT_TRUE(TypeParam().empty());
}

TYPED_TEST(ContainerReqsGeneral, DirectInitUnnamed) {
  EXPECT_THAT(TypeParam(this->a_), testing::ContainerEq(this->a_));
}

TYPED_TEST(ContainerReqsGeneral, DirectInit) {
  TypeParam u(this->a_);
  EXPECT_THAT(u, testing::ContainerEq(this->a_));
}

TYPED_TEST(ContainerReqsGeneral, CopyInit) {
  TypeParam u = this->a_;
  EXPECT_THAT(u, testing::ContainerEq(this->a_));
}

TYPED_TEST(ContainerReqsGeneral, DirectInitMove) {
  TypeParam u(this->rv_);
  EXPECT_THAT(u, testing::ContainerEq(this->r_));
}

TYPED_TEST(ContainerReqsGeneral, CopyInitMove) {
  TypeParam u = this->rv_;
  EXPECT_THAT(u, testing::ContainerEq(this->r_));
}

TYPED_TEST(ContainerReqsGeneral, MoveAssignment) {
  this->a_ = this->rv_;
  EXPECT_THAT(this->a_, testing::ContainerEq(this->r_));
}

TYPED_TEST(ContainerReqsGeneral, Destructor) {
  {
    RingBufAdapter<TypeParam, InstanceCounter, 3> a;
    a.push_back({});
    a.push_back({});

    ASSERT_EQ(InstanceCounter::GetCounter(), 2);
  }
  EXPECT_EQ(InstanceCounter::GetCounter(), 0);
}

TYPED_TEST(ContainerReqsGeneral, BeginEnd) {
  TypeParam a;  // new instance for empty case
  EXPECT_TYPE_EQ(decltype(a.begin()), typename TypeParam::iterator);
  EXPECT_TYPE_EQ(decltype(a.end()), typename TypeParam::iterator);
  EXPECT_EQ(a.begin(), a.end());

  const TypeParam constA;
  EXPECT_TYPE_EQ(decltype(constA.begin()), typename TypeParam::const_iterator);
  EXPECT_TYPE_EQ(decltype(constA.end()), typename TypeParam::const_iterator);
  EXPECT_EQ(constA.begin(), constA.end());
}

TYPED_TEST(ContainerReqsGeneral, CbeginCend) {
  EXPECT_TYPE_EQ(decltype(this->a_.cbegin()),
                 typename TypeParam::const_iterator);
  EXPECT_TYPE_EQ(decltype(this->a_.cend()), typename TypeParam::const_iterator);

  this->a_.clear();
  EXPECT_EQ(this->a_.cbegin(), this->a_.cend());
}

TYPED_TEST(ContainerReqsGeneral, Equality) {
  baudvine::RingBuf<int, 2> a;
  a.push_back(1);
  a.push_back(2);
  a.push_back(3);
  baudvine::RingBuf<int, 2> b;

  EXPECT_TRUE((std::is_convertible<decltype(a == b), bool>::value));
  EXPECT_FALSE(a == b);
  b.push_back(2);
  EXPECT_FALSE(a == b);
  b.push_back(3);
  EXPECT_TRUE(a == b);
}

TYPED_TEST(ContainerReqsGeneral, Inequality) {
  baudvine::RingBuf<int, 2> a;
  a.push_back(1);
  a.push_back(2);
  baudvine::RingBuf<int, 2> b;

  EXPECT_TRUE((std::is_convertible<decltype(a != b), bool>::value));
  EXPECT_TRUE(a != b);
  b.push_back(1);
  b.push_back(2);
  EXPECT_FALSE(a != b);
  b.push_back(3);
  EXPECT_TRUE(a != b);
}

TYPED_TEST(ContainerReqsGeneral, Swap) {
  EXPECT_TRUE(std::is_void<decltype(this->a_.swap(this->b_))>::value);

  auto ax = this->a_;
  auto bx = this->b_;

  this->a_.swap(this->b_);
  EXPECT_THAT(this->a_, testing::Not(testing::ContainerEq(ax)));
  EXPECT_THAT(this->b_, testing::Not(testing::ContainerEq(bx)));
  EXPECT_THAT(this->a_, testing::ContainerEq(bx));
  EXPECT_THAT(this->b_, testing::ContainerEq(ax));
}

TYPED_TEST(ContainerReqsGeneral, StdSwap) {
  EXPECT_TRUE(std::is_void<decltype(std::swap(this->a_, this->b_))>::value);

  auto ax = this->a_;
  auto bx = this->b_;

  std::swap(this->a_, this->b_);
  EXPECT_THAT(this->a_, testing::Not(testing::ContainerEq(ax)));
  EXPECT_THAT(this->b_, testing::Not(testing::ContainerEq(bx)));
  EXPECT_THAT(this->a_, testing::ContainerEq(bx));
  EXPECT_THAT(this->b_, testing::ContainerEq(ax));
}

TYPED_TEST(ContainerReqsGeneral, CopyAssignment) {
  EXPECT_TYPE_EQ(decltype(this->r_ = this->a_), TypeParam&);
  this->r_ = this->a_;
  EXPECT_THAT(this->r_, testing::ContainerEq(this->a_));
}

TYPED_TEST(ContainerReqsGeneral, Size) {
  this->a_.clear();
  EXPECT_EQ(0, std::distance(this->a_.begin(), this->a_.end()));
  EXPECT_EQ(0, this->a_.size());
  this->a_.push_back(10);
  EXPECT_EQ(1, std::distance(this->a_.begin(), this->a_.end()));
  EXPECT_EQ(1, this->a_.size());
  this->a_.push_back(20);
  EXPECT_EQ(2, std::distance(this->a_.begin(), this->a_.end()));
  EXPECT_EQ(2, this->a_.size());
  this->a_.push_back(30);
  EXPECT_EQ(2, std::distance(this->a_.begin(), this->a_.end()));
  EXPECT_EQ(2, this->a_.size());
}

TYPED_TEST(ContainerReqsGeneral, MaxSize) {
  EXPECT_EQ(this->a_.max_size(), 2);
}

TYPED_TEST(ContainerReqsGeneral, Empty) {
  EXPECT_FALSE(this->a_.empty());
  this->a_.clear();
  EXPECT_TRUE(this->a_.empty());
}

TYPED_TEST(ContainerReqsGeneral, IteratorMotion) {
  TypeParam a;

  EXPECT_EQ(a.begin(), a.end());

  a.push_back(4);
  EXPECT_NE(a.begin(), a.end());
  EXPECT_EQ(std::next(a.begin(), 1), a.end());

  a.push_back(3);
  EXPECT_EQ(std::next(a.begin(), 2), a.end());

  a.push_back(2);
  EXPECT_EQ(std::next(a.begin(), 2), a.end());
}

TYPED_TEST(ContainerReqsGeneral, IteratorComparison) {
  EXPECT_TRUE(this->a_.begin() == this->a_.cbegin());
  EXPECT_TRUE(this->a_.cend() == this->a_.end());
  EXPECT_TRUE(this->a_.cbegin() <= this->a_.end());
  EXPECT_TRUE(this->a_.end() > this->a_.cbegin());
}
