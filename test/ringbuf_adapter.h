#pragma once

#include "baudvine/deque_ringbuf.h"
#include "baudvine/flexible_ringbuf.h"
#include "baudvine/ringbuf.h"

// Adapt any ringbuf type T<U, S> to T<Elem, Capacity>. This way the test can be
// parametrized with T<int, 2> but individual tests can change the element type
// and count if necessary.
template <typename T, typename Elem, size_t Capacity>
class RingBufAdapter;

template <typename Elem, size_t Capacity, typename Elem1, size_t Capacity1>
class RingBufAdapter<baudvine::RingBuf<Elem1, Capacity1>, Elem, Capacity>
    : public baudvine::RingBuf<Elem, Capacity> {};

template <typename Elem, size_t Capacity, typename Elem1, size_t Capacity1>
class RingBufAdapter<baudvine::DequeRingBuf<Elem1, Capacity1>, Elem, Capacity>
    : public baudvine::DequeRingBuf<Elem, Capacity> {};

template <typename Elem, size_t Capacity, typename Elem1, size_t Capacity1>
class RingBufAdapter<baudvine::FlexRingBufX<Elem1, Capacity1>, Elem, Capacity>
    : public baudvine::DequeRingBuf<Elem, Capacity> {};
