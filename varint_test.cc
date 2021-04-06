#include "varint.h"

#include <random>

#include "gtest/gtest.h"

TEST(Varint32, Smoke) {
  std::string buf;
  AppendVarint32(42, &buf);
  EXPECT_EQ(buf, "\x54");
  const char* p = buf.data();
  EXPECT_EQ(ParseVarint32(p), 42);
  EXPECT_EQ(p - buf.data(), 1);
}

void RoundTrip(uint32_t x) {
  char buf[kVarint32MaxLength];
  char* encoded = AppendVarint32(x, buf);
  const char* decoded = buf;
  EXPECT_EQ(ParseVarint32(decoded), x);
  EXPECT_EQ(encoded, decoded);
}

TEST(Varint32, Edges) {
  RoundTrip(0);
  RoundTrip(1);

  RoundTrip((uint32_t{1} << 7) - 1);
  RoundTrip((uint32_t{1} << 7));
  RoundTrip((uint32_t{1} << 7) + 1);

  RoundTrip((uint32_t{1} << 14) - 1);
  RoundTrip((uint32_t{1} << 14));
  RoundTrip((uint32_t{1} << 14) + 1);

  RoundTrip((uint32_t{1} << 21) - 1);
  RoundTrip((uint32_t{1} << 21));
  RoundTrip((uint32_t{1} << 21) + 1);

  RoundTrip((uint32_t{1} << 28) - 1);
  RoundTrip((uint32_t{1} << 28));
  RoundTrip((uint32_t{1} << 28) + 1);

  RoundTrip(std::numeric_limits<uint32_t>::max() - 1);
  RoundTrip(std::numeric_limits<uint32_t>::max());
}

void RoundTrip(const std::vector<uint32_t>& seq) {
  std::string buf;
  for (uint32_t x : seq) {
    AppendVarint32(x, &buf);
  }

  std::vector<uint32_t> actual;
  for (const char* p = buf.data(); p != buf.data() + buf.size();) {
    actual.push_back(ParseVarint32(p));
  }

  EXPECT_EQ(actual, seq);
}

TEST(Varint32, Sequence) {
  std::vector<uint32_t> test;
  for (int i = 0; i < 4000000; ++i) {
    test.push_back(i);
  }
  RoundTrip(test);
}

TEST(Varint32, Random) {
  std::random_device rd;
  std::mt19937 e(rd());

  for (int bits = 1; bits <= 32; ++bits) {
    uint32_t max = (uint64_t{1} << bits) - 1;
    std::uniform_int_distribution<uint32_t> dist(0, max);
    std::vector<uint32_t> test;
    for (int i = 0; i < 1000000; ++i) {
      test.push_back(dist(e));
    }
    RoundTrip(test);
  }
}
