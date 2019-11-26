#include "multi-aggregation.h"

namespace {

using AggregatorPtr = std::unique_ptr<AggregatorInterface>;

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

}  // namespace

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
