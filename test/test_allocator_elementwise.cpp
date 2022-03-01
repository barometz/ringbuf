#ifdef BAUDVINE_HAVE_BOOST_IPC
#include "baudvine/ringbuf/deque_ringbuf.h"
#include "baudvine/ringbuf/ringbuf.h"

#include "at_exit.h"
#include "instance_counter.h"
#include "ringbufs.h"

#include <gtest/gtest.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

// Allocator tests using Boost's shared memory allocator.
template <typename RingBuf>
class AllocatorElementwise : public testing::Test {};

namespace ipc = boost::interprocess;

template <typename T>
using Allocator =
    ipc::allocator<T, ipc::managed_shared_memory::segment_manager>;

using RingBufs = AllRingBufs<InstanceCounter, 2, Allocator<InstanceCounter>>;
// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(AllocatorElementwise, RingBufs);

TYPED_TEST(AllocatorElementwise, MoveCtor) {
  std::string name = std::string("Move-") + typeid(TypeParam).name();
  AtExit remove_shmem([&] { ipc::shared_memory_object::remove(name.c_str()); });
  ipc::shared_memory_object::remove(name.c_str());

  ipc::managed_shared_memory shm(ipc::create_only, name.c_str(), 4096);
  TypeParam ringbuf(shm.get_segment_manager());

  ringbuf.emplace_back();
  ringbuf.emplace_back();

  EXPECT_FALSE(ringbuf.front().IsCopy());
  EXPECT_FALSE(ringbuf.front().IsMoved());

  TypeParam moved(std::move(ringbuf));

  // Move ctor always swaps data pointers, so the elements don't get moved.
  EXPECT_FALSE(moved.front().IsCopy());
  EXPECT_FALSE(moved.front().IsMoved());
}

TYPED_TEST(AllocatorElementwise, MoveAssignmentDifferentAlloc) {
  std::string nameA = std::string("MoveA-") + typeid(TypeParam).name();
  std::string nameB = std::string("MoveB-") + typeid(TypeParam).name();
  AtExit remove_shmem([&] {
    ipc::shared_memory_object::remove(nameA.c_str());
    ipc::shared_memory_object::remove(nameB.c_str());
  });
  ipc::shared_memory_object::remove(nameA.c_str());
  ipc::shared_memory_object::remove(nameB.c_str());

  ipc::managed_shared_memory shmA(ipc::create_only, nameA.c_str(), 4096);
  ipc::managed_shared_memory shmB(ipc::create_only, nameB.c_str(), 4096);
  TypeParam ringbufA(shmA.get_segment_manager());
  TypeParam ringbufB(shmB.get_segment_manager());

  ringbufA.emplace_back();
  ringbufB.emplace_back();

  ringbufB = std::move(ringbufA);
  // In the case of move assignment, the elements get moved one by one because
  // the shared memory allocator doesn't transfer over.
  EXPECT_FALSE(ringbufB.front().IsCopy());
  EXPECT_TRUE(ringbufB.front().IsMoved());
}

TYPED_TEST(AllocatorElementwise, MoveAssignmentSameAlloc) {
  std::string name = std::string("MoveSameAlloc-") + typeid(TypeParam).name();
  AtExit remove_shmem([&] { ipc::shared_memory_object::remove(name.c_str()); });
  ipc::shared_memory_object::remove(name.c_str());

  ipc::managed_shared_memory shm(ipc::create_only, name.c_str(), 8092);
  TypeParam ringbufA(shm.get_segment_manager());
  TypeParam ringbufB(shm.get_segment_manager());

  ringbufA.emplace_back();
  ringbufB.emplace_back();

  ringbufB = std::move(ringbufA);
  // In this case, the allocators are the same so no elementwise moves are
  // required.
  EXPECT_FALSE(ringbufB.front().IsCopy());
  EXPECT_FALSE(ringbufB.front().IsMoved());
}

#endif
