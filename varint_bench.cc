#include <benchmark/benchmark.h>
#include <random>

#include "base.h"
#include "varint.h"

std::vector<uint32_t> MakeRandomSeq(int num, int bits) {
  std::random_device rd;
  std::mt19937 e(rd());
  uint32_t max = (uint64_t{1} << bits) - 1;
  std::uniform_int_distribution<uint32_t> dist(0, max);
  std::vector<uint32_t> result;
  for (int i = 0; i < num; ++i) {
    result.push_back(dist(e));
  }
  return result;
}

static void BM_Encode(benchmark::State& state) {
  constexpr int kNumElements = 1000;
  char buf[kNumElements * kVarint32MaxLength];
  std::vector<uint32_t> test = MakeRandomSeq(kNumElements, state.range(0));
  int expected_length = [&] {
    std::string buf;
    for (uint32_t x : test) AppendVarint32(x, &buf);
    return buf.size();
  }();

  uint64_t total_length = 0;
  for (auto _ : state) {
    char* p = buf;
    for (uint32_t x : test) {
      p = AppendVarint32(x, p);
    }
    total_length += p - buf;
  }
  if (total_length != expected_length * state.iterations()) {
    Fail("Bug detected");
  }
  state.SetItemsProcessed(state.iterations() * kNumElements);
}
BENCHMARK(BM_Encode)->Arg(7)->Arg(14)->Arg(21)->Arg(28)->Arg(32);

static void BM_EncodeString(benchmark::State& state) {
  std::string buf;
  std::vector<uint32_t> test = MakeRandomSeq(1000, state.range(0));

  for (auto _ : state) {
    buf.clear();
    for (uint32_t x : test) {
      AppendVarint32(x, &buf);
    }
  }
  state.SetItemsProcessed(state.iterations() * test.size());
}
BENCHMARK(BM_EncodeString)->Arg(7)->Arg(14)->Arg(21)->Arg(28)->Arg(32);

static void BM_Decode(benchmark::State& state) {
  std::string buf;
  std::vector<uint32_t> test = MakeRandomSeq(1000, state.range(0));
  uint32_t checksum = 0;
  for (uint32_t x : test) {
    AppendVarint32(x, &buf);
    checksum += x;
  }

  uint32_t verify = 0;
  const char* begin = buf.data();
  const char* end = begin + buf.size();
  for (auto _ : state) {
    for (const char* p = begin; p != end;) {
      verify += ParseVarint32(p);
    }
  }
  if (static_cast<uint32_t>(checksum * state.iterations()) != verify) {
    Fail("Bug detected");
  }
  state.SetItemsProcessed(state.iterations() * test.size());
}
BENCHMARK(BM_Decode)->Arg(7)->Arg(14)->Arg(21)->Arg(28)->Arg(32);
