#ifndef GITHUB_ZISZIS_ZG_SINGLE_AGGREGATION_INCLUDED
#define GITHUB_ZISZIS_ZG_SINGLE_AGGREGATION_INCLUDED

#include <memory>
#include <vector>

#include "base.h"
#include "table.h"

template <class A>
std::unique_ptr<Table> MakeSingleAggregatorTable(
    const std::vector<AggregationState::Key>& keys, A a);

//===========================================================================
// Implementation below
//===========================================================================

template <class Aggregator, class State>
class SingleAggregatorTable : public Table {
 public:
  SingleAggregatorTable(Aggregator agg, State state)
      : state_(std::move(state)), agg_(std::move(agg)) {}

  void PushRow(const InputRow& row) override {
    state_.Push(
        row, [&] { return agg_.Init(row); },
        [&](typename Aggregator::State& value) { agg_.Update(row, value); });
  }

  void Render() override {
    OutputBuffer buffer;
    state_.Render(&buffer, [this, &buffer](const auto& value) {
      agg_.Print(value, &buffer);
    });
  }

 private:
  State state_;
  Aggregator agg_;
};

template <class A, class State>
auto MakeSingleAggregatorTable(A a, State s) {
  return std::make_unique<SingleAggregatorTable<A, State>>(std::move(a),
                                                           std::move(s));
}

template <class A>
std::unique_ptr<Table> MakeSingleAggregatorTable(
    const std::vector<AggregationState::Key>& keys, A a) {
  if (keys.size() == 0) {
    return MakeSingleAggregatorTable(
        std::move(a), AggregationState::NoKeys<typename A::State>());
  } else if (keys.size() == 1) {
    return MakeSingleAggregatorTable(
        std::move(a), AggregationState::SingleKey<typename A::State>(keys[0]));
  } else {
    return MakeSingleAggregatorTable(
        std::move(a), AggregationState::CompositeKey<typename A::State>(keys));
  }
}

#endif  // GITHUB_ZISZIS_ZG_SINGLE_AGGREGATION_INCLUDED
