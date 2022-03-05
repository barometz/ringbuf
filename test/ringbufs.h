#pragma once

#include "baudvine/deque_ringbuf.h"
#include "baudvine/ringbuf.h"

#include "unflex_ringbuf.h"

#include <gtest/gtest.h>
#include <gtest/internal/gtest-type-util.h>

// All implementations including the adapter.
template <typename T, size_t N, typename Alloc = std::allocator<T>>
using AllRingBufs = testing::Types<baudvine::RingBuf<T, N, Alloc>,
                                   baudvine::DequeRingBuf<T, N, Alloc>,
                                   UnFlexRingBuf<T, N, Alloc>>;

// All completely local implementations.
template <typename T, size_t N, typename Alloc = std::allocator<T>>
using OurRingBufs = testing::Types<baudvine::RingBuf<T, N, Alloc>,
                                   UnFlexRingBuf<T, N, Alloc>>;
