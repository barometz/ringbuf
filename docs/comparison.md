# Comparison to other implementations

## [boost::circular_buffer](https://www.boost.org/doc/libs/1_78_0/doc/html/circular_buffer.html)

Boost's Circular Buffer library is a header-only ring buffer with continuous
backing storage.

* No "spare" element to provide a strong exception guarantee when insertion fails  
  This is intentional, or at least the documentation
  [describes](https://www.boost.org/doc/libs/1_78_0/doc/html/circular_buffer/implementation.html)
  the choice of using copying for overwriting at the end of a full buffer, as
  opposed to `allocator_traits::destroy` and `allocator_traits::construct`.
* No `emplace_front()` or `emplace_back()`
* The capacity is not a template parameter, so it can be resized.
* Supports `insert()` member functions, and declares that elements closer to
  `end()` get knocked off when that overfills the buffer.
* Supports `assign()` member functions.
* Several useful circular-buffer-specific member functions such as
  `linearize()`.
* Rather more extensive documentation.

For simple values (integer samples, for example), `boost::circular_buffer` is
likely more performant than `baudvine::RingBuf`.

## boost::circular_buffer_space_optimized

An adapter around the plain `circular_buffer` with one extra feature: it only
allocates what it needs right now, much like `std::vector` does. If its capacity
is 100 but there are only 2 elements, it probably has 2 elements' worth of
memory allocated.

## [boost::lockfree::spsc_queue](https://www.boost.org/doc/libs/1_78_0/doc/html/lockfree.html)

A lock-free implementation of a single-producer, single-consumer queue with
circular-buffer-like behaviour. This not an STL-style container - it only has
`push()`, `pop()`, and `reset()`, and no notion of an iterator. Which makes
sense, because anything more doesn't really gel with the lock-free feature.

If you have a single producer and a single consumer, and don't need it to play
nice with `<algorithm>`, this is probably a pretty good bet. It does use the
circular buffer concept a little differently: `push()` onto a full queue has no
effect. It also doesn't have vector-like element lifetimes.
