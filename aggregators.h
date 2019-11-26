#ifndef GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
#define GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED

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

class IntSumAggregator {
 public:
  explicit IntSumAggregator(int field) : field_(field) {}

  using State = int64_t;
  State GetDefault() const { return 0; }
  void Push(const InputRow& row, State* state) const {
    *state += row[field_].AsInt64();
  }
  void Print(State state, std::string* out) const {
    absl::StrAppend(out, state);
  }

 private:
  int field_;
};

#endif  // GITHUB_ZISZIS_ZG_AGGREGATORS_INCLUDED
