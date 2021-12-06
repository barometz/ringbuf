add_executable(ringbuf-test)

# Add test sources
file(GLOB test_files "test_*.cpp")
target_sources(ringbuf-test PRIVATE
  examples.cpp
  ${test_files}
  )

target_link_libraries(ringbuf-test PRIVATE baudvine-ringbuf)

set_target_properties(ringbuf-test
  PROPERTIES
  CXX_STANDARD 11
  CXX_EXTENSIONS OFF)

# Wire up googletest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
  )
FetchContent_MakeAvailable(googletest)

# googletest includes a default main() that's good enough here
target_link_libraries(ringbuf-test PRIVATE gtest_main gmock)
include(GoogleTest)
gtest_discover_tests(ringbuf-test)
