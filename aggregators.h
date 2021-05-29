#ifndef GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
#define GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED

#include <limits>
#include <variant>

#include "absl/strings/str_cat.h"
#include "expr.h"
#include "output.h"
#include "storage.h"
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
  bool Min(Numeric);
  bool Max(Numeric);
  void Print(std::string*) const;

 private:
  explicit Numeric(int64_t v) : v_(v) {}
  explicit Numeric(double v) : v_(v) {}
  std::variant<int64_t, double> v_;
};

template<class Value, class R, R(Value::*fn)(Value)>
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
using SumAggregator = GenericAggregator<Value, void, &Value::Add>;

template <class Value>
using MinAggregator = GenericAggregator<Value, bool, &Value::Min>;

template <class Value>
using MaxAggregator = GenericAggregator<Value, bool, &Value::Max>;

template <class Value, bool (Value::*fn)(Value)>
class ArgMAggregator {
 public:
  using State = std::pair<Value, DynamicStorage::Handle>;

  ArgMAggregator(Expr<Value> value,
                 std::vector<ExprColumn<std::string_view>> args)
      : value_(std::move(value)), storage_(std::move(args)) {}

  State Init(const InputRow& row) {
    return {value_.Eval(row), storage_.Store(row)};
  }

  void Update(const InputRow& row, State& state) {
    if ((state.first.*fn)(value_.Eval(row))) {
      storage_.Update(state.second, row);
    }
  }

  void Print(const State& s, OutputTable& out) const {
    storage_.Print(s.second, out);
  }

  void Reset() { storage_.Reset(); }

 private:
  Expr<Value> value_;
  MultiColumnDynamicStorage storage_;
};

template <class Value>
using ArgMinAggregator = ArgMAggregator<Value, &Value::Min>;

template <class Value>
using ArgMaxAggregator = ArgMAggregator<Value, &Value::Max>;

#endif  // GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
