#include <baudvine/ringbuf/ringbuf.h>

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <chrono>
#include <mutex>

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

TEST(Example, ConsoleLog) {
  // Pretend this class has a background task that calls callback_ whenever it
  // receives a line of console output.
  class ConsoleSource {
   public:
    void OnLineReceived(std::function<void(std::string)> callback) {
      callback_ = std::move(callback);
    }

   private:
    std::function<void(std::string)> callback_;
  };

  // We're only interested in about 1024 lines of console log, or maybe we don't
  // have all that much memory to work with.
  baudvine::RingBuf<std::string, 1024> buffer;
  ConsoleSource source{};
  std::mutex bufferMutex;
  // Configure the OnLineReceived handler to add the incoming line to the buffer
  source.OnLineReceived([&buffer, &bufferMutex](std::string line) {
    std::lock_guard lock(bufferMutex);
    buffer.push_front(std::move(line));
  });

  // In the background, ConsoleSource fills up the buffer.

  // Once you want the latest 1024 lines:
  baudvine::RingBuf<std::string, 1024> copy;
  {
    // Copy while holding the lock so ConsoleSource doesn't lock up for too long
    std::lock_guard lock(bufferMutex);
    copy = buffer;
  }
  for (const auto& line : copy) {
    std::cout << line << std::endl;
  }
}

#ifdef BAUDVINE_HAVE_VARIANT
struct SignalEntry {
  SignalEntry(const std::chrono::system_clock::time_point& timestamp,
              const std::string& description,
              const std::variant<int, float, std::string>& value)
      : Timestamp(timestamp), Description(description), Value(value) {}
  std::chrono::system_clock::time_point Timestamp;
  std::string Description;
  std::variant<int, float, std::string> Value;
};

std::ostream& operator<<(std::ostream& os, const SignalEntry& entry) {
  const auto unixTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
      entry.Timestamp.time_since_epoch());
  os << "[" << unixTimestamp.count() << "] " << entry.Description << ": ";
  std::visit([&os](auto v) { os << v; }, entry.Value);
  return os;
}

TEST(Example, SignalHistory) {
  // Imagine you're writing a diagnostics subsystem, and for diagnostic purposes
  // you want to keep a history of all signals that come in over time. But the
  // volume is pretty big, and your system runs for days, so you can't remember
  // everything.

  // So we'll keep 512 entries, because we did the work to determine
  // that this yields about two hours of backlog.
  baudvine::RingBuf<SignalEntry, 512> history;

  // And now you add data as it comes in, and you don't have to worry about the
  // volume so much - you just know that you have the past N entries.
  auto now = std::chrono::system_clock::now;
  history.emplace_front(now(), "Temperature hood [°C]", 67.4f);
  history.emplace_front(now(), "Uptime", "5h1s");
  history.emplace_front(now(), "Temperature hood [°C]", 65.3f);
  history.emplace_front(now(), "Signal strength [dBm]", -25.f);
  history.emplace_front(now(), "Connection count", 12);

  for (const auto& entry : history) {
    std::cout << entry << std::endl;
  }
}
#endif // BAUDVINE_HAVE_VARIANT

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

TEST(Example, Copy) {
  // std::copy is slowish with RingBuf - it has to step through the iterators
  // one by one as it doesn't know there are at worst two contiguous sections.
  // Enter baudvine::copy(), which does.

  baudvine::RingBuf<int, 3> buffer;
  std::vector<int> vec(4);

  buffer.push_back(4);
  buffer.push_back(5);
  buffer.push_back(6);
  buffer.push_back(7);

  baudvine::copy(buffer.begin(), buffer.end(), vec.begin());
  // or more concisely thanks to ADL:
  copy(buffer.begin(), buffer.end(), vec.begin());
  EXPECT_THAT(vec, testing::ElementsAre(5, 6, 7, 0));
}

TEST(Example, StdCopy) {
  baudvine::RingBuf<int, 3> buffer;
  baudvine::RingBuf<int, 4> other;

  buffer.push_back(4);
  buffer.push_back(6);
  buffer.push_back(8);
  buffer.push_back(10);
  buffer.push_back(12);

  other.push_back(7);

  std::copy(buffer.begin(), buffer.end(), std::back_inserter(other));
  EXPECT_THAT(other, testing::ElementsAre(7, 8, 10, 12));
}

TEST(Example, StdSort) {
  baudvine::RingBuf<int, 3> buffer;

  buffer.push_back(10);
  buffer.push_back(4);
  buffer.push_back(12);
  buffer.push_back(8);
  buffer.push_back(6);
  std::sort(buffer.begin(), buffer.end());
  EXPECT_THAT(buffer, testing::ElementsAre(6, 8, 12));
}

TEST(Example, StdRotate) {
  // Rotating a ring buffer doesn't sound useful, but it works!

  baudvine::RingBuf<int, 3> buffer;

  buffer.push_back(4);
  buffer.push_back(6);
  buffer.push_back(8);
  buffer.push_back(10);
  buffer.push_back(12);

  std::rotate(buffer.begin(), buffer.begin() + 1, buffer.end());
  EXPECT_THAT(buffer, testing::ElementsAre(10, 12, 8));
}