This is a header-only
[ring/circular buffer](https://en.wikipedia.org/wiki/Circular_buffer)
implementation in C++11, with the following goals:

- Reasonably performant.
- Readable.
- STL-compatible and STL-like.

There are two implementations: baudvine::RingBuf and baudvine::DequeRingBuf.
They're meant to be functionally equivalent (mostly), but behave differently
under water. Tests ideally cover both, expecting the same behaviour. Both behave
like double-ended queues up until the point where they fill up, at which point
pushing/emplacing at the front causes the element in the back to fall off (and
vice versa).

baudvine::RingBuf is the primary implementation: it stores elements in a
fixed-size, dynamically allocated array, and provides custom iterators as well
as baudvine::copy for efficient copying.

baudvine::DequeRingBuf is included primarily for testing purposes. Because it's
based on `std::deque` it's a bit less efficient (time, memory fragmentation)
than the array-based one, but it's also a lot easier to trust since all the
hairy math and allocation happens in the standard library.

# Usage

```{.cpp}
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

See test/examples.cpp for further usage examples. The high-level pitch is this:
you can use it as an STL container, except it wraps like a ring buffer. You get
iterators, `front()`, `back()`, `push_back()`, `pop_front()`, `emplace_back()`,
range-for, all\* the things you expect from a standard library container.

# Building
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
