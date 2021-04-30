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
  virtual size_t StateAlign() const = 0;
  virtual void Init(const InputRow& row, char* state) const = 0;
  virtual void Update(const InputRow& row, char* state) const = 0;
  virtual void Print(const char* state, OutputTable&) const = 0;
};

template <class A>
std::unique_ptr<AggregatorInterface> TypeErasedAggregator(A a);

std::unique_ptr<Table> MakeMultiAggregatorTable(
    std::vector<Table::Key> keys,
    std::vector<std::unique_ptr<AggregatorInterface>> aggregated_fields,
    std::unique_ptr<OutputTable> output);

//===========================================================================
// Implementation below
//===========================================================================

template <class A>
class AggregatorWrapper : public AggregatorInterface {
 public:
  using State = typename A::State;
  explicit AggregatorWrapper(A a) : a_(std::move(a)) {}
  size_t StateSize() const override { return sizeof(State); }
  size_t StateAlign() const override {
    static_assert(alignof(State) <= 8);
    return alignof(State);
  }
  void Init(const InputRow& row, char* state) const override {
    new (state) State(a_.Init(row));
  }
  void Update(const InputRow& row, char* state) const override {
    a_.Update(row, *reinterpret_cast<State*>(state));
  }
  void Print(const char* state, OutputTable& out) const override {
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
