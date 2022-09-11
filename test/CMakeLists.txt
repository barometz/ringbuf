add_executable(ringbuf-test)

# Add test sources
file(GLOB test_files "test_*.cpp")
target_sources(ringbuf-test PRIVATE
        examples.cpp
        instance_counter.cpp
        black_box.cpp
        ${test_files}
        )

target_link_libraries(ringbuf-test PRIVATE baudvine-ringbuf)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(ringbuf-test PRIVATE
            # Triggers on correct use of TYPED_TEST_SUITE
            -Wno-gnu-zero-variadic-macro-arguments
            )
endif ()

set_target_properties(ringbuf-test
        PROPERTIES
        CXX_EXTENSIONS OFF
        )

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

# Feature detection

find_package(Boost 1.67.0)
if (Boost_FOUND)
    # The allocator-aware tests use Boost's shared memory allocator.
    target_link_libraries(ringbuf-test PRIVATE Boost::headers)
    if (UNIX)
        # need librt for shm_open & co
        target_link_libraries(ringbuf-test PRIVATE rt)
    endif ()
    target_compile_definitions(ringbuf-test PRIVATE BOOST_DATE_TIME_NO_LIB)
    target_compile_definitions(ringbuf-test PRIVATE BAUDVINE_HAVE_BOOST_IPC)
endif ()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
try_compile(have_variant ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/build_tools/feature_check_variant.cpp)
try_compile(have_ranges ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/build_tools/feature_check_ranges.cpp)

target_compile_definitions(ringbuf-test PRIVATE
        $<$<BOOL:${have_variant}>:BAUDVINE_HAVE_VARIANT>
        $<$<BOOL:${have_ranges}>:BAUDVINE_HAVE_RANGES>
        )
