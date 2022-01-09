// Used by CMakeLists.txt to determine whether <ranges> is supported in the
// current build configuration.
#include <ranges>
static_assert(!std::ranges::random_access_range<int>);
