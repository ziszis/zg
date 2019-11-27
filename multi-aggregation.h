#ifndef GITHUB_ZISZIS_ZG_MULTI_AGGREGATION_INCLUDED
#define GITHUB_ZISZIS_ZG_MULTI_AGGREGATION_INCLUDED

#include <memory>
#include <vector>

#include "base.h"
#include "table.h"

// Since generating a Table instantiation for every combination of aggregators
// is not realistic, we type-erase aggregators using this interface.
class AggregatorInterface {
 public:
  virtual ~AggregatorInterface() {}
  virtual size_t StateSize() const = 0;
  virtual void GetDefault(char* storage) const = 0;
  virtual void Push(const InputRow& row, char* state) = 0;
  virtual void Print(const char* state, std::string*) = 0;
};

template <class A>
std::unique_ptr<AggregatorInterface> TypeErasedAggregator(A a);

std::unique_ptr<Table> MakeMultiAggregatorTable(
    const std::vector<int>& key_fields,
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
std::unique_ptr<AggregatorInterface> TypeErasedAggregator(A a) {
  return std::make_unique<AggregatorWrapper<A>>(std::move(a));
}

#endif  // GITHUB_ZISZIS_ZG_MULTI_AGGREGATION_INCLUDED
