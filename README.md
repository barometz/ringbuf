# baudvine/ringbuf
## Overview
This is a header-only
[ring/circular buffer](https://en.wikipedia.org/wiki/Circular_buffer)
implementation in C++11, with the following goals:

- Reasonably performant.
- Readable.
- Roughly STL-compatible.

There are two implementations: `RingBuf` and `DequeRingBuf`. They're meant to be
functionally equivalent (mostly), but behave differently under water. Tests
ideally cover both, expecting the same behaviour.

The Deque variant is included primarily for testing purposes: because it's based
on `std::deque` it's a lot less efficient than the array-based on, but it's also
a lot easier to verify since all the hairy allocation bits happen in the
standard library.

`RingBuf` is the primary implementation: it stores elements in a fixed-size,
dynamically allocated array, adding new elements to the end, and popping
elements off the front when more space is needed.

## Using
See [test/examples.cpp](test/examples.cpp) for usage examples. The high-level
pitch is this: you can use it as an STL container, except it wraps like a ring
buffer. You get iterators, `front()`, `back()`, `push_back()`, `pop_front()`,
`emplace_back()`, range-for, all\* the things you expect from a standard library
container.

## Building
The project is header-only, so you only have to copy
`include/baudvine/ringbuf/ringbuf.h` into your project. You should also be able
to include it as a CMake subproject, but that's untested.

To build and run the tests, install [CMake](https://cmake.org/), then run:

```
mkdir build
cmake -S . -B build
cmake --build build
build/test/ringbuf-test
```

Or use your editor or IDE with CMake integration of choice.

## TODO
What can't it do? Well:

- Reverse iterators.
- There are separate tests for the deque and non-deque iterators.
- Comparison between ring buffers of different capacities isn't possible yet.

## NOT TODO
What won't it do? Most of these will be because of the "Readable" goal. A
typical implementation of std::vector doesn't adhere to this, and if you've ever
tried reading those you know what I mean.

- Don't want to bother with optional features based on C++ version. Strictly C++11.
- No debug-only runtime checks, other than the occasional `assert`.

## Performance

The tests in [test/test_speed.cpp](test/test_speed.cpp) perform a number of
comparisons between `RingBuf` and `DequeRingBuf`. Some typical (but completely
unscientific) results:

- Hardware: Intel Core i5-7600 @ 3.5 GHz
- GCC: 11.2, `-O3 -DNDEBUG`
- Clang: 11.1, `-O3 -DNDEBUG`

| Name | Description | RingBuf (GCC) (ms) | RingBuf (Clang) (ms) | DequeRingBuf (GCC) (ms) | DequeRingBuf (Clang) (ms) |
|------|-------------|-------------------:|---------------------:|------------------------:|--------------------------:|
| PushBackToFull | `push_back` until the buffer is filled to capacity (2<sup>25</sup> elements) | 130 | 60 | 160 | 160 |
| PushBackOverFull | `push_back` 2<sup>25</sup> times on a buffer with capacity 3 | 132 | 63 | 86 | 91 |
| IterateOver | range-for over a buffer with 2<sup>25</sup> elements | 21 | 20 | 48 | 19 |

PushBackToFull is *completely* unfair because `std::deque` has to allocate
memory much more frequently. In a debug build, the results are roughly
proportional (~10x), although `baudvine::RingBuf` does comparatively worse in
IterateOver.

## License
I chose the [MIT license](LICENSE) for this project, because I have a bad habit
of wanting to be useful and MIT's a good one for maximizing that paperclip.
One exception: `deque_ringbuf.h` is MIT-0, which is effectively public domain.
