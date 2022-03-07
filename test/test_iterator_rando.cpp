#include "ringbuf_adapter.h"
#include "ringbufs.h"

#include <gtest/gtest.h>

#include <iterator>

// Functional requirements from [iterator.concepts.random.access]. Static
// concept check is performed in test_container_concepts.cpp.
template <typename RingBuf>
class IteratorRando : public testing::Test {
 protected:
  void SetUp() override {
    // Add some extra so end() is before begin() in the backing storage
    const size_t count = ringbuf_.capacity() + 3;
    for (uint16_t i = 0; i < count; i++) {
      ringbuf_.push_back(i);
    }
    a_ = ringbuf_.begin();
    b_ = ringbuf_.end();
    n_ = ringbuf_.size();
  }

  using D = typename std::iterator_traits<
      typename RingBuf::iterator>::difference_type;

  RingBuf ringbuf_;
  typename RingBuf::iterator a_;
  typename RingBuf::iterator b_;
  D n_;
};

using RingBufs = AllRingBufs<int, 4>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(IteratorRando, RingBufs);

TYPED_TEST(IteratorRando, PlusIs) {
  EXPECT_EQ((this->a_ += this->n_), this->b_);
}

TYPED_TEST(IteratorRando, PlusIsIdentity) {
  const auto* address = std::addressof(this->a_);
  EXPECT_EQ(std::addressof(this->a_ += this->n_), address);
}

TYPED_TEST(IteratorRando, Plus) {
  const auto plus = this->a_ + this->n_;
  const auto plus_is = (this->a_ += this->n_);
  EXPECT_EQ(plus, plus_is);
}

TYPED_TEST(IteratorRando, sulP) {
  EXPECT_EQ(this->a_ + this->n_, this->n_ + this->a_);
}

TYPED_TEST(IteratorRando, PlusZero) {
  using D = typename IteratorRando<TypeParam>::D;
  EXPECT_EQ(this->a_ + D(0), this->a_);
}

TYPED_TEST(IteratorRando, IncrementToEnd) {
  using D = typename IteratorRando<TypeParam>::D;
  EXPECT_EQ((this->a_ + this->n_), [](typename TypeParam::iterator c) {
    return ++c;
  }(this->a_ + D(this->n_ - 1)));
}

TYPED_TEST(IteratorRando, PlusIsNeg) {
  using D = typename IteratorRando<TypeParam>::D;
  EXPECT_EQ((this->b_ += D(-this->n_)), this->a_);
}

TYPED_TEST(IteratorRando, SubIs) {
  EXPECT_EQ((this->b_ -= this->n_), this->a_);
}

TYPED_TEST(IteratorRando, SubIsIdentity) {
  const auto* address = std::addressof(this->b_);
  EXPECT_EQ(std::addressof(this->b_ -= this->n_), address);
}

TYPED_TEST(IteratorRando, Sub) {
  const auto sub = this->b_ - this->n_;
  const auto sub_is = (this->b_ -= this->n_);
  EXPECT_EQ(sub, sub_is);
}

TYPED_TEST(IteratorRando, Subscript) {
  EXPECT_EQ(this->a_[this->n_], *this->b_);
}

TYPED_TEST(IteratorRando, Ordering) {
  RingBufAdapter<TypeParam, int, 2> ring;
  EXPECT_TRUE(ring.begin() == ring.end());
  ring.push_back(1);
  EXPECT_TRUE(ring.begin() < ring.end());
  ring.push_back(2);
  EXPECT_TRUE(ring.begin() < ring.end());
  ring.push_back(3);
  EXPECT_TRUE(ring.begin() < ring.end());
  ring.push_back(4);
  EXPECT_TRUE(ring.begin() < ring.end());
}

TYPED_TEST(IteratorRando, SizedSentinel) {
  EXPECT_EQ(this->b_ - this->a_, this->n_);
  EXPECT_EQ(this->a_ - this->b_, -this->n_);
}
