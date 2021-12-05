#include <baudvine/ringbuf/dynamic_ringbuf.h>
#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>

#include <memory>

enum class Variant {
  Static,
  Dynamic,
};

template <typename Elem, std::size_t Capacity>
class RingBufAdapter {
 public:
  RingBufAdapter(Variant variant) {
    switch (variant) {
      case Variant::Static:
        static_.reset(new baudvine::RingBuf<Elem, Capacity>());
        break;
      case Variant::Dynamic:
        dynamic_.reset(new baudvine::DynamicRingBuf<Elem>(Capacity));
        break;
    }
  }

#define DISPATCH(call)      \
  do {                      \
    if (static_)            \
      return static_->call; \
    return dynamic_->call;  \
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

#undef DISPATCH

 private:
  // TODO: this would be less trouble with std::variant, but the build should
  // also run with C++11.
  std::unique_ptr<baudvine::RingBuf<Elem, Capacity>> static_{};
  std::unique_ptr<baudvine::DynamicRingBuf<Elem>> dynamic_{};
};

class RingBuf : public testing::TestWithParam<Variant> {
 public:
  void SetUp() override {
    under_test_.reset(new RingBufAdapter<std::string, 3>(GetParam()));
  }

  RingBufAdapter<std::string, 3>& GetBuf() { return *under_test_; }

 private:
  std::unique_ptr<RingBufAdapter<std::string, 3>> under_test_;
};

TEST_P(RingBuf, PushBack) {
  auto& underTest = GetBuf();
  underTest.push_back("3");
  underTest.capacity();
}

INSTANTIATE_TEST_SUITE_P(Static, RingBuf, testing::Values(Variant::Static));
INSTANTIATE_TEST_SUITE_P(Dynamic, RingBuf, testing::Values(Variant::Dynamic));
