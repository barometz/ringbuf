#include "baudvine/ringbuf/deque_ringbuf.h"
#include "baudvine/ringbuf/ringbuf.h"

#include <gtest/gtest-typed-test.h>
#include <gtest/gtest.h>

#include <scoped_allocator>

#define EXPECT_TYPE_EQ(T1, T2) EXPECT_TRUE((std::is_same<T1, T2>::value))

template <typename RingBuf>
class ContainerReqsAllocAware : public testing::Test {};

// TODO: separate tests with boost/interprocess/allocators/ (see e.g.
// https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor) and maybe
// foonathan's RawAllocator

template <typename T>
using Allocator = std::scoped_allocator_adaptor<std::allocator<T>>;

using RingBufs = testing::Types<
    baudvine::RingBuf<std::string, 2, Allocator<std::string>>,
    baudvine::DequeRingBuf<std::string, 2, Allocator<std::string>>>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerReqsAllocAware, RingBufs);

TYPED_TEST(ContainerReqsAllocAware, TypeAliases) {
  EXPECT_TYPE_EQ(Allocator<std::string>, typename TypeParam::allocator_type);
}

TYPED_TEST(ContainerReqsAllocAware, GetAllocator) {
  EXPECT_TYPE_EQ(Allocator<std::string>, decltype(TypeParam{}.get_allocator()));
}

#ifdef BAUDVINE_HAVE_BOOST_IPC

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/scope_exit.hpp>

template <typename RingBuf>
class ContainerReqsAllocIpc : public testing::Test {};

// TODO: separate tests with boost/interprocess/allocators/ (see e.g.
// https://en.cppreference.com/w/cpp/memory/scoped_allocator_adaptor) and maybe
// foonathan's RawAllocator

namespace ipc = boost::interprocess;

template <typename T>
using ShmAllocator =
    ipc::allocator<T, ipc::managed_shared_memory::segment_manager>;

using ShmRingBufs = testing::Types<
    baudvine::RingBuf<std::string, 2, ShmAllocator<std::string>>,
    baudvine::DequeRingBuf<std::string, 2, ShmAllocator<std::string>>>;

// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerReqsAllocIpc, ShmRingBufs);

class AtExit {
 public:
  AtExit() = default;
  AtExit(std::function<void()> at_exit) : at_exit_(std::move(at_exit)) {}
  AtExit(const AtExit&) = delete;
  AtExit(AtExit&&) = default;

  ~AtExit() {
    if (at_exit_) {
      at_exit_();
    }
  }

 private:
  std::function<void()> at_exit_;
};

TYPED_TEST(ContainerReqsAllocIpc, Ipc) {
  std::string name = std::string("Ipc-") + typeid(TypeParam).name();

  AtExit remove_shmem([&] { ipc::shared_memory_object::remove(name.c_str()); });
  ipc::shared_memory_object::remove(name.c_str());

  ipc::managed_shared_memory shm(ipc::create_only, name.c_str(), 1024);
  TypeParam underTest(shm.get_segment_manager());
}

#endif
