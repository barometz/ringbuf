add_executable(ringbuf_tests)

# Add test sources
file(GLOB test_files "test_*.cpp")
target_sources(ringbuf_tests PRIVATE ${test_files})

# Wire up googletest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
  )
FetchContent_MakeAvailable(googletest)

# googletest includes a default main() that's good enough here
target_link_libraries(ringbuf_tests gtest_main)

# Enable CTest support for googletest
enable_testing()
include(GoogleTest)
gtest_discover_tests(ringbuf_tests)
