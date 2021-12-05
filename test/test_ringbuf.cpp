#include <baudvine/ringbuf/dynamic_ringbuf.h>
#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>

#include <memory>

namespace {

enum class Variant {
  Static,
  Dynamic,
};

std::ostream& operator<<(std::ostream& os, Variant variant) {
  switch (variant) {
    case Variant::Static:
      return os << "Static";
    case Variant::Dynamic:
      return os << "Dynamic";
  }
}

// Adapter for the two ringbuf implementations, so one set of tests can test
// both (except for iterators). There's probably room for more type erasure, but
// this works fine.
template <typename Elem, std::size_t Capacity>
class RingBufAdapter {
 public:
  RingBufAdapter(Variant variant) : variant_(variant) {
    switch (variant) {
      case Variant::Static:
        static_ = baudvine::RingBuf<Elem, Capacity>();
        break;
      case Variant::Dynamic:
        dynamic_ = baudvine::DynamicRingBuf<Elem>(Capacity);
        break;
    }
  }

#define DISPATCH(call)         \
  do {                         \
    switch (variant_) {        \
      case (Variant::Static):  \
        return static_.call;   \
      case (Variant::Dynamic): \
        return dynamic_.call;  \
    }                          \
  } while (false)

  Elem& operator[](size_t index) { DISPATCH(operator[](index)); }
  const Elem& operator[](size_t index) const { DISPATCH(operator[](index)); }
  Elem& at(size_t index) { DISPATCH(at(index)); }
  const Elem& at(size_t index) const { DISPATCH(at(index)); }
  void push_back(const Elem& value) { DISPATCH(push_back(value)); }
  void pop_front() { DISPATCH(pop_front()); }
  size_t size() { DISPATCH(size()); }
  size_t capacity() { DISPATCH(capacity()); }
  size_t max_size() { DISPATCH(max_size()); }
  bool empty() { DISPATCH(empty()); }

  friend bool operator<(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    switch (lhs.variant_) {
      case (Variant::Static):
        return lhs.static_ < rhs.static_;
      case (Variant::Dynamic):
        return lhs.dynamic_ < rhs.dynamic_;
    }
  }

  friend bool operator==(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    switch (lhs.variant_) {
      case (Variant::Static):
        return lhs.static_ == rhs.static_;
      case (Variant::Dynamic):
        return lhs.dynamic_ == rhs.dynamic_;
    }
  }

  friend bool operator!=(const RingBufAdapter& lhs, const RingBufAdapter& rhs) {
    switch (lhs.variant_) {
      case (Variant::Static):
        return lhs.static_ != rhs.static_;
      case (Variant::Dynamic):
        return lhs.dynamic_ != rhs.dynamic_;
    }
  }

  void swap(RingBufAdapter& other) {
    switch (variant_) {
      case (Variant::Static):
        return static_.swap(other);
      case (Variant::Dynamic):
        return dynamic_.swap(other);
    }
  }

#undef DISPATCH

 private:
  Variant variant_;

  // This would be less annoying with std::variant, but the build should also
  // run with C++11.
  baudvine::RingBuf<Elem, Capacity> static_{};
  baudvine::DynamicRingBuf<Elem> dynamic_{};
};

class RingBuf : public testing::TestWithParam<Variant> {
 public:
  template <typename Elem, size_t Capacity>
  RingBufAdapter<Elem, Capacity> MakeAdapter() {
    return RingBufAdapter<Elem, Capacity>(GetParam());
  }
};
}  // namespace

TEST_P(RingBuf, Zero) {
  // The degenerate case of a size zero buffer can still essentially work, it
  // just doesn't do anything useful. Consistency is king.
  auto underTest = MakeAdapter<int, 0>();

  EXPECT_EQ(underTest.capacity(), 0);
  EXPECT_EQ(underTest.size(), 0);
  EXPECT_NO_THROW(underTest.push_back(53));
  EXPECT_EQ(underTest.size(), 0);
}

TEST_P(RingBuf, Capacity) {
  EXPECT_EQ((MakeAdapter<char, 128>()).capacity(), 128);
  EXPECT_EQ((MakeAdapter<int, 1>()).capacity(), 1);
  EXPECT_EQ((MakeAdapter<int, 128>()).capacity(), 128);
  EXPECT_EQ((MakeAdapter<int, 500>()).capacity(), 500);
}

TEST_P(RingBuf, AtEmpty) {
  auto underTest = MakeAdapter<int, 4>();

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST_P(RingBuf, AtConstEmpty) {
  const auto underTest = MakeAdapter<int, 4>();

  EXPECT_THROW(underTest.at(0), std::out_of_range);
  EXPECT_THROW(underTest.at(1), std::out_of_range);
  EXPECT_THROW(underTest.at(4), std::out_of_range);
}

TEST_P(RingBuf, PushBack) {
  auto underTest = MakeAdapter<std::string, 3>();
  underTest.push_back("one");
  EXPECT_EQ(underTest.at(0), "one");
  underTest.push_back("two");
  EXPECT_EQ(underTest.at(1), "two");
  EXPECT_EQ(underTest.at(0), "one");
  EXPECT_EQ(underTest.size(), 2);
}

TEST_P(RingBuf, PushOver) {
  auto underTest = MakeAdapter<std::string, 2>();
  underTest.push_back("one");
  underTest.push_back("two");
  underTest.push_back("three");

  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), "two");
  EXPECT_EQ(underTest.at(1), "three");

  underTest.push_back("five");
  underTest.push_back("six");
  underTest.push_back("seven");
  EXPECT_EQ(underTest.size(), 2);
  EXPECT_EQ(underTest.at(0), "six");
  EXPECT_EQ(underTest.at(1), "seven");
}

TEST_P(RingBuf, Pop) {
  auto underTest = MakeAdapter<int, 3>();
  underTest.push_back(41);
  underTest.pop_front();
  EXPECT_TRUE(underTest.empty());
  EXPECT_EQ(underTest.size(), 0);

  for (auto i : {1, 2}) {
    std::ignore = i;
    underTest.push_back(42);  // push
    underTest.push_back(43);  // push
    underTest.push_back(44);  // push
    underTest.push_back(45);  // push, 42 rolls off
    underTest.pop_front();    // pop, 43 rolls off

    EXPECT_EQ(underTest.at(0), 44);
    EXPECT_EQ(underTest.size(), 2);
  }
}

// Tests for https://en.cppreference.com/w/cpp/named_req/Container
using Container = RingBuf;

TEST_P(Container, CopyCtor) {
  auto original = MakeAdapter<std::string, 2>();
  original.push_back("zero");
  original.push_back("one");
  original.push_back("two");

  RingBufAdapter<std::string, 2> copy(original);
  EXPECT_EQ(copy.at(0), "one");
  EXPECT_EQ(copy.at(1), "two");
}

TEST_P(Container, MoveCtor) {
  auto original = MakeAdapter<std::string, 2>();
  original.push_back("zero");
  original.push_back("one");
  original.push_back("two");

  RingBufAdapter<std::string, 2> moved(original);
  EXPECT_EQ(moved.at(0), "one");
  EXPECT_EQ(moved.at(1), "two");
}

TEST_P(Container, Assignment) {
  auto original = MakeAdapter<std::string, 2>();
  original.push_back("zero");
  original.push_back("one");
  original.push_back("two");

  auto copy = MakeAdapter<std::string, 2>();
  copy = original;
  EXPECT_EQ(copy.at(0), "one");
  EXPECT_EQ(copy.at(1), "two");
}

TEST_P(Container, MoveAssignment) {
  auto original = MakeAdapter<std::string, 2>();
  original.push_back("zero");
  original.push_back("one");
  original.push_back("two");

  auto copy = MakeAdapter<std::string, 2>();
  copy = std::move(original);
  EXPECT_EQ(copy.at(0), "one");
  EXPECT_EQ(copy.at(1), "two");
}

TEST_P(Container, Equality) {
  auto a = MakeAdapter<int, 3>();
  auto b = MakeAdapter<int, 3>();
  EXPECT_EQ(a, b);

  a.push_back(2010);
  a.push_back(3030);
  EXPECT_NE(a, b);

  b.push_back(2010);
  b.push_back(3030);
  EXPECT_EQ(a, b);

  b.push_back(4070);
  EXPECT_NE(a, b);
}

TEST_P(Container, Size) {
  auto underTest = MakeAdapter<float, 3>();
  EXPECT_EQ(underTest.size(), 0);

  underTest.push_back(0.4f);
  EXPECT_EQ(underTest.size(), 1);

  underTest.push_back(0.0f);
  underTest.push_back(2.4e9f);
  underTest.push_back(5.0e9f);
  EXPECT_EQ(underTest.size(), 3);
}

TEST_P(Container, MaxSize) {
  auto underTest = MakeAdapter<float, 3>();
  EXPECT_GE(underTest.max_size(), 3);
}

TEST_P(Container, Empty) {
  auto underTest = MakeAdapter<double, 2>();
  EXPECT_TRUE(underTest.empty());
  underTest.push_back(0);
  EXPECT_FALSE(underTest.empty());
  underTest.push_back(1.0);
  EXPECT_FALSE(underTest.empty());
  underTest.push_back(2.0);
  EXPECT_FALSE(underTest.empty());

  baudvine::RingBuf<double, 0> alwaysEmpty;
  EXPECT_TRUE(alwaysEmpty.empty());
  underTest.push_back(0.1);
  EXPECT_TRUE(alwaysEmpty.empty());
}

TEST_P(Container, Swap) {
  baudvine::RingBuf<int, 3> a;
  baudvine::RingBuf<int, 3> b;

  a.push_back(2010);
  a.push_back(3030);

  b.push_back(4500);
  b.push_back(20);
  b.push_back(9999);

  auto a2 = a;
  auto b2 = b;
  a2.swap(b2);

  EXPECT_EQ(a, b2);
  EXPECT_EQ(b, a2);
}

INSTANTIATE_TEST_SUITE_P(RingBuf,
                         RingBuf,
                         testing::Values(Variant::Static, Variant::Dynamic));
INSTANTIATE_TEST_SUITE_P(Container,
                         Container,
                         testing::Values(Variant::Static, Variant::Dynamic));
