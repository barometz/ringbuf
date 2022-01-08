# baudvine::RingBuf

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

For detailed documentation, see https://barometz.github.io/ringbuf/.

This is a header-only
[ring/circular buffer](https://en.wikipedia.org/wiki/Circular_buffer)
implementation in C++11, with the following goals:

- Reasonably performant.
- Readable.
- STL-compatible and STL-like.

There are two implementations: `baudvine::RingBuf` and `baudvine::DequeRingBuf`.
They're meant to be functionally equivalent (mostly), but behave differently
under water. Tests ideally cover both, expecting the same behaviour. Both behave
like double-ended queues up until the point where they fill up, at which point
pushing/emplacing at the front causes the element in the back to fall off (and
vice versa).

`baudvine::RingBuf` is the primary implementation: it stores elements in a
fixed-size, dynamically allocated array, and provides the usual container
operations as well as `baudvine::copy` for efficient copying.

`baudvine::DequeRingBuf` is included primarily for testing purposes. Because
it's based on `std::deque` it's a bit less efficient (time, memory
fragmentation) than the array-based one, but it's also a lot easier to trust
since all the hairy math and allocation happens in the standard library.

## The present

### Using

```c++
#include <baudvine/ringbuf/ringbuf.h>

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

- [x] Support for user-defined/provided allocators.
- [x] Reverse iterators.
- [ ] Generic tests for the iterators of the different implementations.
- [ ] Doesn't quite implement
  [SequenceContainer](https://en.cppreference.com/w/cpp/named_req/SequenceContainer).
- [x] The tests for Container aren't as well organized as they could be either.
- [x] Needs more ~~cowbell~~ `noexcept`
- [x] Moved-from RingBuf instances can't be reused.
- [ ] The iterator is not std::random_access_iterator, which is a slightly dubious but plausible fit.
- [x] Docs.
- [ ] Better code examples.

### NOT TODO
What won't it do?

- Don't want to bother with optional features based on C++ version. Strictly C++11.
- Runtime checks are limited to the occasional `assert`.

### MAYBE TODO

- [x] It's meant to be C++11-compatible without a lot of optional chunks for
  newer standards, but maybe we can show that it *does* match the relevant C++20
  iterator concepts.
- [ ] An implementation with mutable capacity (drop the template parameter, add
  a ctor parameter and a `set_capacity` member) would be slightly slower and
  slightly nicer to use.
- [ ] DequeRingBuf could possibly be a container adapter.
- [ ] A variant of RingBuf with automatic storage duration (so no dynamic
  allocation) is possible. Would be nice for embedded targets.

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

A functional requirement of `baudvine::Ringbuf` is that element lifetime is
controlled by the `push` and `pop` member functions: a T is constructed when it
is pushed into the buffer, and it is destroyed when popped. The tests using the
`Instance` class in `tests/test_ringbuf.cpp` demonstrate this.

Neither C-style arrays nor `std::array` allow for this:
`std::array<std::string, 5>` default-initializes five strings (calling its
default constructor), and if you destroy it in-place it'll get re-destroyed when
the array goes out of scope.

See also https://barometz.github.io/ringbuf/design-notes.html#memory-allocation.

### Why isn't this an adapter for existing containers?

No underlying container would provide all three of: 

- Single allocation (`std::array`, `std::vector`)
- Push and pop on opposing ends (`std::deque`, `std::list`)
- Control over element lifetime (`std::vector`, `std::list`, `std::deque`)

See also: https://barometz.github.io/ringbuf/design-notes.html#not-an-adapter.

## License
I chose the [MIT license](LICENSE) for this project, because I have a bad habit
of wanting to be useful and MIT's a good one for maximizing that paperclip.
One exception: `deque_ringbuf.h` is MIT-0, which is effectively public domain.
