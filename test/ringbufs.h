#pragma once

#include <baudvine/ringbuf/deque_ringbuf.h>
#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>
#include <gtest/internal/gtest-type-util.h>

template <typename T, size_t N>
using AllRingBufs =
    testing::Types<baudvine::RingBuf<T, N>, baudvine::DequeRingBuf<T, N>>;
