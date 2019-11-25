#include <memory>
#include <string>
#include <string_view>

#include "absl/container/flat_hash_map.h"
#include "aggregators.h"
#include "base.h"
#include "input.h"
#include "types.h"

class OutputBuffer {
 public:
  std::string* raw() { return &buf_; }
  void MaybeFlush() {
    if (buf_.size() > 1 << 15) Flush();
  }
  ~OutputBuffer() { Flush(); }

 private:
  void Flush() {
    if (std::fwrite(buf_.data(), 1, buf_.size(), stdout) != buf_.size()) {
      Fail("Write failed");
    }
    buf_.clear();
  }

  std::string buf_;
};

class Table {
 public:
  virtual ~Table() {}
  virtual void PushRow(const InputRow& row) = 0;
  virtual void Render() = 0;
};

template <class Aggregator>
class NoGroupingTable : public Table {
 public:
  explicit NoGroupingTable(Aggregator agg = Aggregator()) : agg_(agg) {
    agg_.Init(&state_);
  }

  void PushRow(const InputRow& row) override { agg_.Push(row, &state_); }

  void Render() override {
    OutputBuffer buffer;
    agg_.Print(state_, buffer.raw());
    buffer.raw()->push_back('\n');
  }

 private:
  typename Aggregator::State state_;
  int value_field_;
  Aggregator agg_;
};

template <class Aggregator>
class SingleAggregatorTable : public Table {
 public:
  SingleAggregatorTable(int key_field, Aggregator agg = Aggregator())
      : key_field_(key_field), agg_(agg) {}

  void PushRow(const InputRow& row) override {
    auto [it, inserted] = rows_.try_emplace(row[key_field_].AsString());
    if (inserted) {
      agg_.Init(&it->second);
    }
    agg_.Push(row, &it->second);
  }

  void Render() override {
    OutputBuffer buffer;
    for (const auto& [key, value] : rows_) {
      buffer.raw()->append(key);
      buffer.raw()->push_back('\t');
      agg_.Print(value, buffer.raw());
      buffer.raw()->push_back('\n');
      buffer.MaybeFlush();
    }
  }

 private:
  absl::flat_hash_map<std::string, typename Aggregator::State> rows_;
  int key_field_;
  Aggregator agg_;
};

std::unique_ptr<Table> ParseSpec(absl::string_view spec) {
  if (spec == "k2c") {
    return std::make_unique<SingleAggregatorTable<CountAggregator>>(1);
  } else if (spec == "k1c") {
    return std::make_unique<SingleAggregatorTable<CountAggregator>>(0);
  } else if (spec == "k2s1") {
    return std::make_unique<SingleAggregatorTable<IntSumAggregator>>(
        1, IntSumAggregator(0));
  } else if (spec == "c") {
    return std::make_unique<NoGroupingTable<CountAggregator>>();
  } else {
    Fail("Unsupported spec");
    return nullptr;
  }
}

int main(int argc, char* argv[]) {
  InputRow row;
  if (argc < 2) Fail("Need spec");
  std::unique_ptr<Table> table = ParseSpec(argv[1]);
  ForEachInputLine([&](const char* begin, const char* end) {
    SplitLine(begin, end, &row);
    table->PushRow(row);
  });
  table->Render();
  return 0;
}
