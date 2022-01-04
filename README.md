# baudvine/ringbuf

- [Overview](#overview)
- [The present](#the-present)
    - [Using](#using)
    - [Building](#building)
- [The Future](#the-future)
    - [TODO](#todo)
    - [NOT TODO](#not-todo)
    - [MAYBE TODO](#maybe-todo)
- [Performance](#performance)
- [FAQ](#faq)
- [License](#license)

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

## The present

### Using

```c++
#include <baudvine/ringbuf.h>

void demo()
{
  baudvine::RingBuf<std::string, 2> buffer;
  buffer.push_back("Testing, ");
  buffer.push_back("one, ");
  buffer.push_back("two.");
  assert(buffer.front() == "one, ");
  assert(buffer.back() == "two.");
}
```

See [test/examples.cpp](test/examples.cpp) for further usage examples. The
high-level pitch is this: you can use it as an STL container, except it wraps
like a ring buffer. You get iterators, `front()`, `back()`, `push_back()`,
`pop_front()`, `emplace_back()`, range-for, all\* the things you expect from a
standard library container.

### Building
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

## The Future

### TODO
What can't it do? Well:

- Support for user-defined/provided allocators.
  It already uses `std::allocator` so it shouldn't require any big changes, just
  small and careful ones.
- Reverse iterators.
- Generic tests for the iterators of the different implementations.
- Doesn't quite implement
  [SequenceContainer](https://en.cppreference.com/w/cpp/named_req/SequenceContainer).
  The tests for Container aren't as well organized as they could be either.
- Needs more ~~cowbell~~ `noexcept`
- Moved-from RingBuf instances can't be reused.
- The iterator is not std::random_access_iterator, which is a slightly dubious but plausible fit.

### NOT TODO
What won't it do?

- Don't want to bother with optional features based on C++ version. Strictly C++11.
- Runtime checks are limited to the occasional `assert`.

### MAYBE TODO

- It's meant to be C++11-compatible without a lot of optional chunks for newer
  standards, but maybe we can show that it *does* match the relevant C++20
  iterator concepts.
- An implementation with mutable capacity (drop the template parameter, add a
  ctor parameter and a `set_capacity` member) would be slightly slower and
  slightly nicer to use.

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

## FAQ

### Why does this allocate "manually" and not just wrap `std::array`?
A functional requirement of `baudvine::ringbuf` is that element lifetime is
controlled by the `push` and `pop` member functions: a T is constructed when it
is pushed into the buffer, and it is destroyed when popped. The tests using the
`RefCounter` class in `tests/test_ringbuf.cpp` demonstrate this.

With `std::array`, it'd be possible to perform in-place construction and
destruction, but once the `std::array` is destroyed all of its elements get
destroyed as well - which breaks when not all elements are valid objects.

### Why isn't this an adapter for existing containers?
The `baudvine::DequeRingBuf` class is very nearly an adapter on top of
`std::deque`, so it's reasonable to ask why the entire implementation doesn't
simply wrap an arbitrary container. The trouble is that no existing container
yields exactly the properties I'm looking for.

- `std::vector` gets you `push_back` and `pop_back` for free, but for
  `pop_front` you need to either lose elementwise lifetimes or move all values
  by one position.
- `std::deque` (as demonstrated) gets you all the member functions for free but
  not the compile-time size. It also can't allocate the entire capacity in one
  chunk, increasing memory fragmentation.
- `std::list` is the same as `std::deque`, except with per-element allocation
  which leads to more fragmentation and slower iteration.
- `std::array` is a single allocation but you lose elementwise lifetimes, as
  discussed above.

## License
I chose the [MIT license](LICENSE) for this project, because I have a bad habit
of wanting to be useful and MIT's a good one for maximizing that paperclip.
One exception: `deque_ringbuf.h` is MIT-0, which is effectively public domain.
