#include <ranges>
static_assert(!std::ranges::random_access_range<int>);
