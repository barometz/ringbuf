cmake_minimum_required(VERSION 3.14)
project(baudvine-ringbuf)

add_library(baudvine-ringbuf INTERFACE)
target_include_directories(baudvine-ringbuf INTERFACE include)

add_subdirectory(test)
