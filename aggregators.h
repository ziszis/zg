#ifndef GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
#define GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED

#include <limits>
#include <variant>

#include "absl/strings/str_cat.h"
#include "types.h"

class CountAggregator {
 public:
  using State = int64_t;
  State GetDefault() const { return 0; }
  void Push(const InputRow&, State* state) const { ++*state; }
  void Print(State state, std::string* out) const {
    absl::StrAppend(out, state);
  }
};

class Numeric {
 public:
  void Add(const FieldValue&);
  void Min(const FieldValue&);
  void Max(const FieldValue&);
  void Print(std::string*) const;

  static Numeric MinValue() { return {}; }
  static Numeric MaxValue() { return {}; }

 private:
  std::variant<std::monostate, int64_t, double> v_;
};

template <class V>
class NativeNum {
 public:
  NativeNum() : v_(0) {}
  void Add(const FieldValue& field) { v_ += Cast<V>(field); }
  void Min(const FieldValue& field) { v_ = std::min(v_, Cast<V>(field)); }
  void Max(const FieldValue& field) { v_ = std::max(v_, Cast<V>(field)); }
  void Print(std::string* out) const { absl::StrAppend(out, v_); }

  static NativeNum<V> MinValue() {
    return NativeNum(std::numeric_limits<V>::lowest());
  }
  static NativeNum<V> MaxValue() {
    return NativeNum(std::numeric_limits<V>::max());
  }

 private:
  V v_;

  explicit NativeNum(V v) : v_(v) {}
};

template <class Value>
class SumAggregator {
 public:
  using State = Value;
  explicit SumAggregator(int field) : field_(field) {}
  State GetDefault() const { return State(); }
  void Push(const InputRow& row, State* s) const { s->Add(row[field_]); }
  void Print(State s, std::string* out) const { s.Print(out); }

 private:
  int field_;
};

template <class Value>
class MinAggregator {
 public:
  using State = Value;
  explicit MinAggregator(int field) : field_(field) {}
  State GetDefault() const { return Value::MaxValue(); }
  void Push(const InputRow& row, State* s) const { s->Min(row[field_]); }
  void Print(State s, std::string* out) const { s.Print(out); }

 private:
  int field_;
};

template <class Value>
class MaxAggregator {
 public:
  using State = Value;
  explicit MaxAggregator(int field) : field_(field) {}
  State GetDefault() const { return Value::MinValue(); }
  void Push(const InputRow& row, State* s) const { s->Max(row[field_]); }
  void Print(State s, std::string* out) const { s.Print(out); }

 private:
  int field_;
};

#endif  // GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
