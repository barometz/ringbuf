// Used by CMakeLists.txt to determine whether std::variant is supported in the
// current build configuration.
#include <variant>
std::variant<int, char> test;
