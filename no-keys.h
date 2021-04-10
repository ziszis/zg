#ifndef GITHUB_ZISZIS_ZG_NO_KEY_INCLUDED
#define GITHUB_ZISZIS_ZG_NO_KEY_INCLUDED

#include <optional>

#include "table.h"

template <class Aggregator>
class NoKeyTable : public Table {
 public:
  explicit NoKeyTable(Aggregator aggregator)
      : aggregator_(std::move(aggregator)) {}

  void PushRow(const InputRow& row) override {
    if (value_) {
      aggregator_.Update(row, *value_);
    } else {
      value_ = aggregator_.Init(row);
    }
  }

  void Render(OutputBuffer* out) const override {
    if (value_) {
      aggregator_.Print(*value_, out);
      out->EndLine();
    } else {
      Fail("No data to aggregate");
    }
  }

 private:
  std::optional<typename Aggregator::State> value_;
  Aggregator aggregator_;
};

#endif  // GITHUB_ZISZIS_ZG_NO_KEY_INCLUDED
