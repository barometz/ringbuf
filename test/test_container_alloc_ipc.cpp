#ifdef BAUDVINE_HAVE_BOOST_IPC
#include "at_exit.h"
#include "ringbufs.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include <scoped_allocator>

// Allocator tests using Boost's shared memory allocator.
template <typename RingBuf>
class ContainerReqsAllocIpc : public testing::Test {};

namespace ipc = boost::interprocess;

template <typename T>
using Allocator =
    ipc::allocator<T, ipc::managed_shared_memory::segment_manager>;

// In a real application, this would be
// std::scoped_allocator<
//  Allocator<ipc::basic_string<char, std::char_traits<char>, Allocator<char>>>,
//  Allocator<char>>
// so the string contents are also allocated in the shared memory. The generated
// test names are long enough as it is.
using RingBufs = AllRingBufs<std::string, 2, Allocator<std::string>>;

// NOLINTNEXTLINE - clang-tidy complains about missing variadic args
TYPED_TEST_SUITE(ContainerReqsAllocIpc, RingBufs);

TEST(ContainerReqsAllocIpcScoped, Create) {
  // one-off, demonstrate that it works with scoped_allocator.
  // This does *not* work with std::string, but it's fine with (for example) a
  // vector<vector<int>>. For some reason you need boost's IPC string type to
  // make this work.
  using Elem = ipc::basic_string<char, std::char_traits<char>, Allocator<char>>;
  using ScopedAllocator =
      std::scoped_allocator_adaptor<Allocator<Elem>, Allocator<int>>;
  using RingBuf = baudvine::RingBuf<Elem, 4, ScopedAllocator>;

  std::string name = std::string("Ipc-Scoped");
  ipc::shared_memory_object::remove(name.c_str());
  AtExit remove_shmem([&] { ipc::shared_memory_object::remove(name.c_str()); });
  ipc::managed_shared_memory shm(ipc::create_only, name.c_str(), 1024);

  RingBuf buffer(
      ScopedAllocator(shm.get_segment_manager(), shm.get_segment_manager()));
  EXPECT_EQ(buffer.emplace_back("Jaffa"), "Jaffa");
  EXPECT_EQ(buffer.emplace_back("Kree"), "Kree");
  EXPECT_EQ(buffer[0].get_allocator().get_segment_manager(),
            shm.get_segment_manager());
}

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

TYPED_TEST(ContainerReqsAllocIpc, AllocExtendedCopyCtor) {
  std::string nameA = std::string("AECopyA-") + typeid(TypeParam).name();
  std::string nameB = std::string("AECopyB-") + typeid(TypeParam).name();
  AtExit remove_shmem([&] {
    ipc::shared_memory_object::remove(nameA.c_str());
    ipc::shared_memory_object::remove(nameB.c_str());
  });
  ipc::shared_memory_object::remove(nameA.c_str());
  ipc::shared_memory_object::remove(nameB.c_str());

  ipc::managed_shared_memory shmA(ipc::create_only, nameA.c_str(), 4096);
  ipc::managed_shared_memory shmB(ipc::create_only, nameB.c_str(), 4096);
  TypeParam ringbuf(shmA.get_segment_manager());

  TypeParam copy(ringbuf, shmB.get_segment_manager());
  EXPECT_EQ(Allocator<std::string>(shmB.get_segment_manager()),
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
TYPED_TEST(ContainerReqsAllocIpc, AllocExtendedMoveCtor) {
  std::string nameA = std::string("AEMoveA-") + typeid(TypeParam).name();
  std::string nameB = std::string("AEMoveB-") + typeid(TypeParam).name();
  AtExit remove_shmem([&] {
    ipc::shared_memory_object::remove(nameA.c_str());
    ipc::shared_memory_object::remove(nameB.c_str());
  });
  ipc::shared_memory_object::remove(nameA.c_str());
  ipc::shared_memory_object::remove(nameB.c_str());

  ipc::managed_shared_memory shmA(ipc::create_only, nameA.c_str(), 4096);
  ipc::managed_shared_memory shmB(ipc::create_only, nameB.c_str(), 4096);
  TypeParam ringbuf(shmA.get_segment_manager());

  TypeParam moved(std::move(ringbuf), shmB.get_segment_manager());
  EXPECT_EQ(Allocator<std::string>(shmB.get_segment_manager()),
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

  ringbufB = std::move(ringbufA);
  // Allocator doesn't get copied in this case.
  EXPECT_EQ(Allocator<std::string>(shmB.get_segment_manager()),
            ringbufB.get_allocator());

  ringbufB.push_back("john");
  EXPECT_THAT(ringbufB, testing::ElementsAre("paul", "john"));
}

#endif
