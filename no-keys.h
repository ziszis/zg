#ifndef GITHUB_ZISZIS_ZG_NO_KEY_INCLUDED
#define GITHUB_ZISZIS_ZG_NO_KEY_INCLUDED

#include <optional>

#include "table.h"

template <class Aggregator>
class NoKeyTable : public Table {
 public:
  NoKeyTable(Aggregator aggregator, std::unique_ptr<OutputTable> output)
      : aggregator_(std::move(aggregator)), output_(std::move(output)) {}

  void PushRow(const InputRow& row) override {
    if (value_) {
      aggregator_.Update(row, *value_);
    } else {
      value_ = aggregator_.Init(row);
    }
  }

  void Finish() override {
    if (value_) {
      aggregator_.Print(*value_, *output_);
      output_->EndLine();
      value_.reset();
      aggregator_.Reset();
      output_->Finish();
    } else {
      Fail("No data to aggregate");
    }
  }

 private:
  std::optional<typename Aggregator::State> value_;
  Aggregator aggregator_;
  std::unique_ptr<OutputTable> output_;
};

#endif  // GITHUB_ZISZIS_ZG_NO_KEY_INCLUDED
