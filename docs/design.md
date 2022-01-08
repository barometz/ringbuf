# Design notes {#design-notes}

## STL compatibility {#stl-compat}

baudvine::RingBuf is meant to be STL-compatible and STL-like. To that end, most
of its functionality is defined and tested with an eye on C++20's general
requirements for standard containers such as `std::vector` and `std::deque`. Not
everything applies - for example, all standard containers other than
`std::array` may allocate during their lifetime, whereas baudvine::RingBuf only
allocates during construction.

RingBuf is a container
([container.requirements.general/4](https://timsong-cpp.github.io/cppwp/n4868/container.requirements.general#4)),
with the exception of the spaceship comparison operator (`<=>`). It is a
container of elements with begin and end iterators, which can be copied, moved
and swapped with other instances of the same type.

RingBuf is a reversible container
([container.requirements.general/10](https://timsong-cpp.github.io/cppwp/n4868/container.requirements.general#10)),
which means all begin/end member functions have reverse counterparts for
iterating backwards.

RingBuf is an allocator-aware container
([container.requirements.general/17](https://timsong-cpp.github.io/cppwp/n4868/container.requirements.general#17)),
meaning that it's possible to supply your own memory allocator (a typical
example is the `boost::interprocess` [shared memory
manager](https://www.boost.org/doc/libs/1_78_0/doc/html/interprocess/quick_guide.html#interprocess.quick_guide.qg_interprocess_container)).
The ring buffer's backing storage as well as that of any contained
allocator-aware objects (such as strings) can then be allocated through that
allocator.

RingBuf is mostly a sequence container
([sequence.reqmts/4](https://timsong-cpp.github.io/cppwp/n4868/sequence.reqmts#4)),
and thoroughly defining and testing that is next on the list.

## Memory allocation {#memory-allocation}

RingBuf uses dynamic memory allocation, or colloquially heap allocation. This is
not strictly required, but automatic ("stack") allocation would require more
work.

Element lifetime is tied to the push, emplace and pop member functions. A
declaration of `baudvine::RingBuf<std::string, 5> ring;` does not instantiate
any strings: the elements are constructed on `push_back`, and destroyed on
`pop_front` (or clear(), or RingBuf destruction). Dynamic allocation makes this
simple: `std::allocator<std::string>::allocate(65);` allocates memory for 65
strings but does not initialize any of them, allowing the container to control
element lifetimes. Automatic storage is more difficult.

The expression `std::string arr[65]` will result in 65 default-initialized
strings. Since `std::string` has a default constructor, that means 65 strings
will be initialized. For some types that doesn't matter, but for others you
really want to avoid running 65 (or a million) default constructors. And if the
type *doesn't* have a default constructor you can't create that array at all.

The way to do this with automatic storage duration involves aligned storage and
allocating space for byte arrays the size of the elements. An implementation
like that may be added in the future, and maybe even become the norm.

## Allocation size {#allocation-size}

To provide a [strong exception
guarantee](https://en.cppreference.com/w/cpp/language/exceptions#Exception_safety),
RingBuf allocates one element more than `Capacity`. When the ring is full and
`emplace_back` is called, the new element is created in the free space between
the last and first elements. Only when construction succeeds is pop_front called
and the ring moved around. As a result, when construction throws an exception,
everything stays the same.

## Not an adapter {#not-an-adapter}

A recurring question (twice, so far) is "why is this not a container adapter,
like std::queue?". A container adapter wraps an existing container to provide
more specialized functionality. For example, `std::stack` adapts another
container (`std::deque` by default) and provides `pop()` and `push()` based on
its `pop_back()` and `push_back()` members. The baudvine::DequeRingBuf is almost
that, and may yet be updated to allow for different inner containers.

However, there is no other container that provides all three of: 

- Single allocation (`std::array`, `std::vector`)
- Push and pop on opposing ends (`std::deque`, `std::list`)
- Control over element lifetime (`std::vector`, `std::list`, `std::deque`)

... whereas baudvine::RingBuf does. "Single allocation" is the least important
of the three, but for embedded allocations with limited memory it's common to
allocate everything upfront so you're sure you won't run out.
