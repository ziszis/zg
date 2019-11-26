#include <memory>
#include <string>
#include <string_view>

#include "aggregators.h"
#include "base.h"
#include "input.h"
#include "output.h"
#include "table.h"
#include "types.h"

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

template <class A, class StateFactory>
auto MakeSingleAggregatorTable(A a, StateFactory sf) {
  auto state = sf(a.GetDefault());
  return std::make_unique<SingleAggregatorTable<A, decltype(state)>>(
      std::move(a), std::move(state));
}

template <class A>
std::unique_ptr<Table> MakeSingleAggregatorTable(
    const std::vector<int>& key_fields, A a) {
  if (key_fields.size() == 0) {
    return MakeSingleAggregatorTable(std::move(a),
                                     AggregationState::NoKeyFactory());
  } else if (key_fields.size() == 1) {
    return MakeSingleAggregatorTable(
        std::move(a), AggregationState::OneKeyFactory{key_fields[0]});
  } else {
    Fail("Multiple grouping keys not supported yet");
    return nullptr;
  }
}

class AggregatorInterface {
 public:
  virtual ~AggregatorInterface() {}
  virtual size_t StateSize() const = 0;
  virtual void GetDefault(char* storage) const = 0;
  virtual void Push(const InputRow& row, char* state) = 0;
  virtual void Print(const char* state, std::string*) = 0;
};
using AggregatorPtr = std::unique_ptr<AggregatorInterface>;

template <class A>
class AggregatorWrapper : public AggregatorInterface {
 public:
  using State = typename A::State;
  explicit AggregatorWrapper(A a) : a_(std::move(a)) {}
  size_t StateSize() const override {
    constexpr size_t size = alignof(State);
    static_assert((size & (size - 1)) == 0);
    return size;
  }
  void GetDefault(char* storage) const override {
    *reinterpret_cast<State*>(storage) = a_.GetDefault();
  }
  void Push(const InputRow& row, char* state) override {
    a_.Push(row, reinterpret_cast<State*>(state));
  }
  void Print(const char* state, std::string* out) override {
    a_.Print(*reinterpret_cast<const State*>(state), out);
  }

 private:
  A a_;
};

template <class A>
std::unique_ptr<AggregatorInterface> WrapAggregator(A a) {
  return std::make_unique<AggregatorWrapper<A>>(std::move(a));
}

struct AggregatorField {
  AggregatorPtr aggregator;
  size_t state_offset;
};

template <class State>
class MultiAggregatorTable : public Table {
 public:
  MultiAggregatorTable(State state, std::vector<AggregatorField> fields)
      : state_(std::move(state)), fields_(std::move(fields)) {}

  void PushRow(const InputRow& row) override {
    char* state = state_.state(row);
    for (const auto& f : fields_) {
      f.aggregator->Push(row, state + f.state_offset);
    }
  }

  void Render() override {
    OutputBuffer buffer;
    state_.Render(&buffer, [this, &buffer](const char* state) {
      for (const auto& f : fields_) {
        f.aggregator->Print(state + f.state_offset, buffer.raw());
      }
    });
  }

 private:
  State state_;
  std::vector<AggregatorField> fields_;
};

template <class S>
auto MakeMultiAggregatorTable(S s, std::vector<AggregatorField> fields) {
  return std::make_unique<MultiAggregatorTable<S>>(std::move(s),
                                                   std::move(fields));
}

template <class StateFactory, int state_size>
std::unique_ptr<Table> MakeMultiAggregatorTableWithSize(
    StateFactory state_factory, std::vector<AggregatorField> fields) {
  char dflt[state_size];
  for (const auto& f : fields) f.aggregator->GetDefault(dflt + f.state_offset);
  return MakeMultiAggregatorTable(state_factory(std::move(dflt)),
                                  std::move(fields));
}

std::pair<size_t, std::vector<AggregatorField>> LayoutAggregatorState(
    std::vector<AggregatorPtr> aggs) {
  std::vector<AggregatorField> fields;
  for (AggregatorPtr& agg : aggs) {
    fields.emplace_back();
    fields.back().state_offset = agg->StateSize();
    fields.back().aggregator = std::move(agg);
  }
  std::sort(fields.begin(), fields.end(), [](const auto& a, const auto& b) {
    return a.state_offset > b.state_offset;
  });
  size_t size = 0;
  for (auto& f : fields) {
    size_t this_size = f.state_offset;
    f.state_offset += size;
    size += this_size;
  }
  return {size, std::move(fields)};
}

template <class StateFactory>
std::unique_ptr<Table> MakeMultiAggregatorTableWithStateFactory(
    StateFactory state_factory, std::vector<AggregatorPtr> aggs) {
  auto [total_size, fields] = LayoutAggregatorState(std::move(aggs));

  if (total_size <= 8) {
    return MakeMultiAggregatorTableWithSize<StateFactory, 8>(
        std::move(state_factory), std::move(fields));
  } else if (total_size <= 16) {
    return MakeMultiAggregatorTableWithSize<StateFactory, 16>(
        std::move(state_factory), std::move(fields));
  } else {
    Fail("Too much state");  // Add more branches.
    return nullptr;
  }
}

std::unique_ptr<Table> MakeMultiAggregatorTable(
    const std::vector<int>& key_fields, std::vector<AggregatorPtr> fields) {
  if (key_fields.size() == 0) {
    return MakeMultiAggregatorTableWithStateFactory(
        AggregationState::NoKeyFactory(), std::move(fields));
  } else if (key_fields.size() == 1) {
    return MakeMultiAggregatorTableWithStateFactory(
        AggregationState::OneKeyFactory{key_fields[0]}, std::move(fields));
  } else {
    Fail("Multiple grouping keys not supported yet");
    return nullptr;
  }
}

template <class Fn>
void ParseSpec(char* spec[], std::vector<int>* key_fields, Fn fn) {
  key_fields->clear();
  if (spec[1] == std::string("k2c")) {
    key_fields->push_back(1);
    fn(CountAggregator());
  } else if (spec[1] == std::string("k1c")) {
    key_fields->push_back(0);
    fn(CountAggregator());
  } else if (spec[1] == std::string("k2s1")) {
    key_fields->push_back(1);
    fn(IntSumAggregator(0));
  } else if (spec[1] == std::string("c")) {
    fn(CountAggregator());
  } else {
    Fail("Unsupported spec");
  }
}

std::unique_ptr<Table> ParseSpec(char* spec[]) {
  std::vector<int> key_fields;

  int num_agg = 0;
  ParseSpec(spec, &key_fields, [&](auto) { ++num_agg; });

  std::unique_ptr<Table> result;
  if (num_agg == 0) {
    Fail("Not implemented");
  } else if (num_agg == 1) {
    ParseSpec(spec, &key_fields, [&](auto agg) {
      result = MakeSingleAggregatorTable(key_fields, std::move(agg));
    });
  } else {
    std::vector<AggregatorPtr> aggs;
    ParseSpec(spec, &key_fields, [&](auto agg) {
      aggs.push_back(WrapAggregator(std::move(agg)));
    });
    result = MakeMultiAggregatorTable(key_fields, std::move(aggs));
  }
  return result;
}

int main(int argc, char* argv[]) {
  InputRow row;
  if (argc < 2) Fail("Need spec");
  std::unique_ptr<Table> table = ParseSpec(argv);
  ForEachInputLine([&](const char* begin, const char* end) {
    SplitLine(begin, end, &row);
    table->PushRow(row);
  });
  table->Render();
  return 0;
}
