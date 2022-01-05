#pragma once

#include <baudvine/ringbuf/deque_ringbuf.h>
#include <baudvine/ringbuf/ringbuf.h>

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
