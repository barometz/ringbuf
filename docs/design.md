# Design notes

## STL compatibility

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

## Memory allocation

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
will be initialized. WHICH MEANS WHAT

## Allocation size

To provide a [strong exception guarantee](https://en.cppreference.com/w/cpp/language/exceptions#Exception_safety), 
RingBuf allocates one element more than WHAT

## Not an adapter

WHAT