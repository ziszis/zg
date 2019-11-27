#ifndef GITHUB_ZISZIS_ZG_SINGLE_AGGREGATION_INCLUDED
#define GITHUB_ZISZIS_ZG_SINGLE_AGGREGATION_INCLUDED

#include <memory>
#include <vector>

#include "base.h"
#include "table.h"

template <class A>
std::unique_ptr<Table> MakeSingleAggregatorTable(
    const std::vector<int>& key_fields, A a);

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

#endif  // GITHUB_ZISZIS_ZG_SINGLE_AGGREGATION_INCLUDED
