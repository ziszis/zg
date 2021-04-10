#ifndef GITHUB_ZISZIS_ZG_MULTI_AGGREGATION_INCLUDED
#define GITHUB_ZISZIS_ZG_MULTI_AGGREGATION_INCLUDED

#include <memory>
#include <vector>

#include "base.h"
#include "output.h"
#include "table.h"

// Since generating a Table instantiation for every combination of aggregators
// is not realistic, we type-erase aggregators using this interface.
class AggregatorInterface {
 public:
  virtual ~AggregatorInterface() {}
  virtual size_t StateSize() const = 0;
  virtual void Init(const InputRow& row, char* state) const = 0;
  virtual void Update(const InputRow& row, char* state) const = 0;
  virtual void Print(const char* state, OutputBuffer*) const = 0;
};

template <class A>
std::unique_ptr<AggregatorInterface> TypeErasedAggregator(A a);

std::unique_ptr<Table> MakeMultiAggregatorTable(
    const std::vector<AggregationState::Key>& keys,
    std::vector<std::unique_ptr<AggregatorInterface>> aggregated_fields);

//===========================================================================
// Implementation below
//===========================================================================

template <class A>
class AggregatorWrapper : public AggregatorInterface {
 public:
  using State = typename A::State;
  explicit AggregatorWrapper(A a) : a_(std::move(a)) {}
  size_t StateSize() const override {
    constexpr size_t size = sizeof(State);
    // Aggregator state layout depends on state sizes of individual agg
    static_assert((size & (size - 1)) == 0);
    static_assert(size % alignof(State) == 0);
    return size;
  }
  void Init(const InputRow& row, char* state) const override {
    *reinterpret_cast<State*>(state) = a_.Init(row);
  }
  void Update(const InputRow& row, char* state) const override {
    a_.Update(row, *reinterpret_cast<State*>(state));
  }
  void Print(const char* state, OutputBuffer* out) const override {
    a_.Print(*reinterpret_cast<const State*>(state), out);
  }

 private:
  A a_;
};

template <class A>
std::unique_ptr<AggregatorInterface> TypeErasedAggregator(A a) {
  static_assert(std::is_trivially_destructible<typename A::State>::value);
  return std::make_unique<AggregatorWrapper<A>>(std::move(a));
}

#endif  // GITHUB_ZISZIS_ZG_MULTI_AGGREGATION_INCLUDED
