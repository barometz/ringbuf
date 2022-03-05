cmake_minimum_required(VERSION 3.14)
project(baudvine-ringbuf)

# Build options
option(RINGBUF_TESTS "Build the baudvine-ringbuf tests" ON)

# Library configuration. Header-only, so "interface"
add_library(baudvine-ringbuf INTERFACE)
target_include_directories(baudvine-ringbuf INTERFACE include)
target_compile_features(baudvine-ringbuf INTERFACE cxx_std_11)

if (MSVC)
    add_compile_options(
            /W4
            # C4127 "conditional expression is constant" - don't have `if constexpr` in c++11
            /wd4127
    )
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror=return-type)
endif ()

# Install configuration
set_target_properties(baudvine-ringbuf
        PROPERTIES
        PUBLIC_HEADER include/baudvine/ringbuf/ringbuf.h
        )
include(GNUInstallDirs)
install(
        TARGETS baudvine-ringbuf
        PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/baudvine/ringbuf/
)

# Build the tests (or not)
if (${RINGBUF_TESTS})
    enable_testing()
    add_subdirectory(test)
endif ()
