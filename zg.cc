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

class AggregationState {
 public:
  template<class Value>
  class NoKey {
   public:
    explicit NoKey(Value value) : value_(std::move(value)) {}
    Value& state(const InputRow&) { return value_; }

    template <class Fn>
    void Render(OutputBuffer* out, Fn fn) const {
      fn(value_);
    }

   private:
    Value value_;
  };

  template <class Value>
  class OneKey {
   public:
    OneKey(int key_field, Value default_value)
        : key_field_(key_field), default_(std::move(default_value)) {}

    Value& state(const InputRow& input) {
      return state_.try_emplace(input[key_field_].AsString(), default_)
          .first->second;
    }

    template <class Fn>
    void Render(OutputBuffer* out, Fn fn) const {
      for (const auto& [key, value] : state_) {
        out->raw()->append(key);
        out->raw()->push_back('\t');
        fn(value);
        out->raw()->push_back('\n');
        out->MaybeFlush();
      }
    }

   private:
    absl::flat_hash_map<std::string, Value> state_;
    int key_field_;
    Value default_;
  };

 private:
};

class Table {
 public:
  virtual ~Table() {}
  virtual void PushRow(const InputRow& row) = 0;
  virtual void Render() = 0;
};

template <class Aggregator, class State>
class SingleAggregatorTable : public Table {
 public:
  SingleAggregatorTable(Aggregator agg, State state)
      : state_(std::move(state)), agg_(std::move(agg)) {}

  void PushRow(const InputRow& row) override {
    agg_.Push(row, &state_.state(row));
  }

  void Render() override {
    OutputBuffer buffer;
    state_.Render(&buffer, [this, &buffer](const auto& value) {
      agg_.Print(value, buffer.raw());
    });
  }

 private:
  State state_;
  Aggregator agg_;
};

template <class A, class S>
auto MakeSingleAggregatorTable(A a, S s) {
  return std::make_unique<SingleAggregatorTable<A, S>>(std::move(a),
                                                       std::move(s));
}

std::unique_ptr<Table> ParseSpec(absl::string_view spec) {
  if (spec == "k2c") {
    return MakeSingleAggregatorTable(
        CountAggregator(),
        AggregationState::OneKey<CountAggregator::State>(1, 0));
  } else if (spec == "k1c") {
    return MakeSingleAggregatorTable(
        CountAggregator(),
        AggregationState::OneKey<CountAggregator::State>(0, 0));
  } else if (spec == "k2s1") {
    return MakeSingleAggregatorTable(
        IntSumAggregator(0),
        AggregationState::OneKey<IntSumAggregator::State>(1, 0));
  } else if (spec == "c") {
    return MakeSingleAggregatorTable(
        CountAggregator(), AggregationState::NoKey<CountAggregator::State>(0));
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
