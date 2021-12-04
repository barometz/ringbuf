#include <gtest/gtest.h>

#include <baudvine/ringbuf/ringbuf.h>

TEST(Create, Create)
{
  baudvine::ringbuf<int, 128>{};
}
