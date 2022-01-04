#pragma once

// While the RingBuf implementation is free of version-dependent implementation
// details, the tests need to work on C++11 and all newer versions, and it's
// okay for some of the tests to only work on e.g. C++17.

// First determine the language version.

#define BAUDVINE_CXX14 201402L
#define BAUDVINE_CXX17 201703L
#define BAUDVINE_CXX20 202002L

#ifdef _MSVC_LANG
#define BAUDVINE_CPLUSPLUS _MSVC_LANG
#else
#define BAUDVINE_CPLUSPLUS __cplusplus
#endif

#if BAUDVINE_CPLUSPLUS >= BAUDVINE_CXX14
#define BAUDVINE_HAVE_CXX14 1
#endif
#if BAUDVINE_CPLUSPLUS >= BAUDVINE_CXX17
#define BAUDVINE_HAVE_CXX17 1
#endif
#if BAUDVINE_CPLUSPLUS >= BAUDVINE_CXX20
#define BAUDVINE_HAVE_CXX20 1
#endif

// Feature tests:

#ifdef BAUDVINE_HAVE_CXX17
#define BAUDVINE_HAVE_VARIANT 1
#endif
