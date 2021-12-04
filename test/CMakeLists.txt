add_executable(ringbuf_tests)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
  )
FetchContent_MakeAvailable(googletest)

target_link_libraries(ringbuf_tests gtest_main)

target_sources(ringbuf_tests PRIVATE test.cpp)

enable_testing()
include(GoogleTest)
gtest_discover_tests(ringbuf_tests)
