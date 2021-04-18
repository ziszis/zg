#ifndef GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
#define GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED

#include <limits>
#include <variant>

#include "absl/strings/str_cat.h"
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

 private:
  int column_;
  mutable std::string buf_;
};

class Numeric {
 public:
  static Numeric Make(FieldValue);

  void Add(FieldValue);
  void Min(FieldValue);
  void Max(FieldValue);
  void Print(std::string*) const;

 private:
  explicit Numeric(int64_t v) : v_(v) {}
  explicit Numeric(double v) : v_(v) {}
  std::variant<int64_t, double> v_;
};

template <class V>
class NativeNum {
 public:
  static NativeNum Make(const FieldValue& field) {
    return NativeNum(ParseAs<V>(field));
  }

  void Add(const FieldValue& field) { v_ += ParseAs<V>(field); }
  void Min(const FieldValue& field) { v_ = std::min(v_, ParseAs<V>(field)); }
  void Max(const FieldValue& field) { v_ = std::max(v_, ParseAs<V>(field)); }
  void Print(std::string* out) const { absl::StrAppend(out, v_); }

 private:
  explicit NativeNum(V v) : v_(v) {}
  V v_;
};

template <class Value>
class SumAggregator {
 public:
  using State = Value;
  SumAggregator(int field, int column) : field_(field), column_(column) {}

  State Init(const InputRow& row) const { return State::Make(row[field_]); }
  void Update(const InputRow& row, State& state) const {
    state.Add(row[field_]);
  }
  void Print(State s, OutputTable& out) const {
    buf_.clear();
    s.Print(&buf_);
    out.Set(column_, buf_);
  }

 private:
  int field_;
  int column_;
  mutable std::string buf_;
};

template <class Value>
class MinAggregator {
 public:
  using State = Value;
  MinAggregator(int field, int column) : field_(field), column_(column) {}
  State Init(const InputRow& row) const { return State::Make(row[field_]); }
  void Update(const InputRow& row, State& state) const {
    state.Min(row[field_]);
  }
  void Print(State s, OutputTable& out) const {
    buf_.clear();
    s.Print(&buf_);
    out.Set(column_, buf_);
  }

 private:
  int field_;
  int column_;
  mutable std::string buf_;
};

template <class Value>
class MaxAggregator {
 public:
  using State = Value;
  MaxAggregator(int field, int column) : field_(field), column_(column) {}
  State Init(const InputRow& row) const { return State::Make(row[field_]); }
  void Update(const InputRow& row, State& state) const {
    state.Max(row[field_]);
  }
  void Print(State s, OutputTable& out) const {
    buf_.clear();
    s.Print(&buf_);
    out.Set(column_, buf_);
  }

 private:
  int field_;
  int column_;
  mutable std::string buf_;
};

#endif  // GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
