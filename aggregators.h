#ifndef GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
#define GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED

#include <limits>
#include <variant>

#include "absl/strings/str_cat.h"
#include "expr.h"
#include "output.h"
#include "types.h"

class CountAggregator {
 public:
  explicit CountAggregator(int column) : column_(column) {}
  using State = int64_t;
  State Init(const InputRow&) const { return 1; }
  void Update(const InputRow&, State& state) const { ++state; }
  void Print(State state, OutputTable& out) const {
    buf_.clear();
    absl::StrAppend(&buf_, state);
    out.Set(column_, buf_);
  }
  void Reset() const { decltype(buf_)().swap(buf_); }

 private:
  int column_;
  mutable std::string buf_;
};

class Numeric {
 public:
  static Numeric Make(FieldValue);

  void Add(Numeric);
  void Min(Numeric);
  void Max(Numeric);
  void Print(std::string*) const;

 private:
  explicit Numeric(int64_t v) : v_(v) {}
  explicit Numeric(double v) : v_(v) {}
  std::variant<int64_t, double> v_;
};

template<class Value, void(Value::*fn)(Value)>
class GenericAggregator {
 public:
  using State = Value;
  explicit GenericAggregator(ExprColumn<Value> expr) : expr_(expr) {}

  State Init(const InputRow& row) const { return expr_.Eval(row); }
  void Update(const InputRow& row, State& state) const {
    (state.*fn)(expr_.Eval(row));
  }
  void Print(State s, OutputTable& out) const { expr_.Print(s, out); }
  void Reset() const { expr_.Reset(); }

 private:
  ExprColumn<Value> expr_;
};

template <class Value>
using SumAggregator = GenericAggregator<Value, &Value::Add>;

template <class Value>
using MinAggregator = GenericAggregator<Value, &Value::Min>;

template <class Value>
using MaxAggregator = GenericAggregator<Value, &Value::Max>;



#endif  // GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
