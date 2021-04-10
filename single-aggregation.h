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
    agg_.Push(row, &state_.state(row));
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

template <class A, class StateFactory>
auto MakeSingleAggregatorTable(A a, StateFactory sf) {
  auto state = sf(a.GetDefault());
  return std::make_unique<SingleAggregatorTable<A, decltype(state)>>(
      std::move(a), std::move(state));
}

template <class A>
std::unique_ptr<Table> MakeSingleAggregatorTable(
    const std::vector<AggregationState::Key>& keys, A a) {
  if (keys.size() == 0) {
    return MakeSingleAggregatorTable(std::move(a),
                                     AggregationState::NoKeyFactory());
  } else if (keys.size() == 1) {
    return MakeSingleAggregatorTable(std::move(a),
                                     AggregationState::OneKeyFactory{keys[0]});
  } else {
    return MakeSingleAggregatorTable(std::move(a),
                                     AggregationState::MultiKeyFactory{keys});
  }
}

#endif  // GITHUB_ZISZIS_ZG_SINGLE_AGGREGATION_INCLUDED
