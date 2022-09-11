#pragma once

#include <cstdint>

// Some functions that do very little but should prevent the compiler from
// optimizing the entire speed test away.

uint64_t black_box(uint64_t input);
void do_nothing(const void* nothing);
