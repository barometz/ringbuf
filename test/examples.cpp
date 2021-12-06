#include <baudvine/ringbuf/ringbuf.h>

#include <gtest/gtest.h>

#include <gmock/gmock-matchers.h>

TEST(Example, PushPop) {
  baudvine::RingBuf<std::string, 3> buffer;

  // A circular buffer lets you keep adding elements, but once it exceeds its
  // configured size (3, in this case) the oldest one gets removed.
  buffer.push_back("line 1");
  EXPECT_EQ(buffer.front(), "line 1");

  buffer.push_back("another line");
  EXPECT_THAT(buffer, testing::ElementsAre("line 1", "another line"));

  buffer.push_back("and they just keep coming");
  // So once you push the fourth element to the back...
  buffer.push_back("and they won't stop coming");
  // The first element falls out.
  EXPECT_THAT(buffer,
              testing::ElementsAre("another line", "and they just keep coming",
                                   "and they won't stop coming"));

  EXPECT_EQ(buffer.front(), "another line");
  EXPECT_EQ(buffer.back(), "and they won't stop coming");
}

TEST(Example, Indexing) {
  baudvine::RingBuf<int, 3> buffer;

  buffer.push_back(5);
  buffer.push_back(4);
  buffer.push_back(3);
  buffer.push_back(2);
  buffer.pop_front();

  EXPECT_EQ(buffer[0], 3);
  EXPECT_EQ(buffer.at(1), 2);
  EXPECT_THROW(buffer.at(2), std::out_of_range);
}

TEST(Example, RangeFor) {
  baudvine::RingBuf<int, 4> buffer;

  buffer.push_back(5);
  buffer.push_back(4);
  buffer.push_back(3);
  buffer.push_back(2);

  int expected = 5;
  for (auto& value : buffer) {
    EXPECT_EQ(value, expected);
    expected--;
  }
}
