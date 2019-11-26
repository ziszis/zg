#ifndef GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
#define GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED

#include <limits>

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

template <class Value>
class SumAggregator {
 public:
  explicit SumAggregator(int field) : field_(field) {}

  using State = Value;
  State GetDefault() const { return 0; }
  void Push(const InputRow& row, State* state) const {
    *state += row[field_].template As<Value>();
  }
  void Print(State state, std::string* out) const {
    absl::StrAppend(out, state);
  }

 private:
  int field_;
};

template <class Value>
class MinAggregator {
 public:
  explicit MinAggregator(int field) : field_(field) {}

  using State = Value;
  State GetDefault() const { return std::numeric_limits<Value>::max(); }
  void Push(const InputRow& row, State* state) const {
    *state = std::min(*state, row[field_].template As<Value>());
  }
  void Print(State state, std::string* out) const {
    absl::StrAppend(out, state);
  }

 private:
  int field_;
};

template <class Value>
class MaxAggregator {
 public:
  explicit MaxAggregator(int field) : field_(field) {}

  using State = Value;
  State GetDefault() const { return std::numeric_limits<Value>::min(); }
  void Push(const InputRow& row, State* state) const {
    *state = std::max(*state, row[field_].template As<Value>());
  }
  void Print(State state, std::string* out) const {
    absl::StrAppend(out, state);
  }

 private:
  int field_;
};

#endif  // GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
