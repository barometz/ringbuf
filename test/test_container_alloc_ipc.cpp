#ifdef BAUDVINE_HAVE_BOOST_IPC
#include "baudvine/ringbuf/deque_ringbuf.h"
#include "baudvine/ringbuf/ringbuf.h"

#include "at_exit.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

// Allocator tests using Boost's shared memory allocator.
template <typename RingBuf>
class ContainerReqsAllocIpc : public testing::Test {};

namespace ipc = boost::interprocess;

template <typename T>
using Allocator =
    ipc::allocator<T, ipc::managed_shared_memory::segment_manager>;

// In a real application, std::string would also need that allocator so the
// actual string ends up in shared memory as well - but the generated test names
// are long enough as it is.
using RingBufs = testing::Types<
    baudvine::RingBuf<std::string, 2, Allocator<std::string>>,
    baudvine::DequeRingBuf<std::string, 2, Allocator<std::string>>>;

// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerReqsAllocIpc, RingBufs);

TYPED_TEST(ContainerReqsAllocIpc, Create) {
  std::string name = std::string("Ipc-") + typeid(TypeParam).name();
  AtExit remove_shmem([&] { ipc::shared_memory_object::remove(name.c_str()); });
  ipc::shared_memory_object::remove(name.c_str());

  ipc::managed_shared_memory shm(ipc::create_only, name.c_str(), 1024);
  TypeParam ringbuf(shm.get_segment_manager());
  EXPECT_EQ(Allocator<std::string>(shm.get_segment_manager()),
            ringbuf.get_allocator());

  ringbuf.push_back("zoinks");
  EXPECT_EQ(ringbuf.front(), "zoinks");
}

TYPED_TEST(ContainerReqsAllocIpc, CopyCtor) {
  std::string name = std::string("Copy-") + typeid(TypeParam).name();
  AtExit remove_shmem([&] { ipc::shared_memory_object::remove(name.c_str()); });
  ipc::shared_memory_object::remove(name.c_str());

  ipc::managed_shared_memory shm(ipc::create_only, name.c_str(), 4096);
  TypeParam ringbuf(shm.get_segment_manager());

  TypeParam copy(ringbuf);
  EXPECT_EQ(Allocator<std::string>(shm.get_segment_manager()),
            copy.get_allocator());
}

TYPED_TEST(ContainerReqsAllocIpc, MoveCtor) {
  std::string name = std::string("IpcMove-") + typeid(TypeParam).name();
  AtExit remove_shmem([&] { ipc::shared_memory_object::remove(name.c_str()); });
  ipc::shared_memory_object::remove(name.c_str());

  ipc::managed_shared_memory shm(ipc::create_only, name.c_str(), 4096);
  TypeParam ringbuf(shm.get_segment_manager());

  TypeParam moved(std::move(ringbuf));
  EXPECT_EQ(Allocator<std::string>(shm.get_segment_manager()),
            moved.get_allocator());
}

TYPED_TEST(ContainerReqsAllocIpc, CopyAssignment) {
  std::string nameA = std::string("CopyA-") + typeid(TypeParam).name();
  std::string nameB = std::string("CopyB-") + typeid(TypeParam).name();
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

  ringbufB = ringbufA;
  // Allocator doesn't get copied in this case.
  EXPECT_EQ(Allocator<std::string>(shmB.get_segment_manager()),
            ringbufB.get_allocator());
}

TYPED_TEST(ContainerReqsAllocIpc, MoveAssignment) {
  std::string nameA = std::string("IpcMoveA-") + typeid(TypeParam).name();
  std::string nameB = std::string("IpcMoveB-") + typeid(TypeParam).name();
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

  ringbufA.push_back("ringo");
  ringbufA.push_back("paul");
  ringbufB.push_back("pete");
  ringbufB.push_back("tommy");

  // Pretty sure this should do elementwise move?
  ringbufB = std::move(ringbufA);
  // Allocator doesn't get copied in this case.
  EXPECT_EQ(Allocator<std::string>(shmB.get_segment_manager()),
            ringbufB.get_allocator());

  ringbufB.push_back("john");
  EXPECT_THAT(ringbufB, testing::ElementsAre("paul", "john"));
}

#endif
