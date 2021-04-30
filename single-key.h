#ifndef GITHUB_ZISZIS_ZG_SINGLE_KEY_INCLUDED
#define GITHUB_ZISZIS_ZG_SINGLE_KEY_INCLUDED

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "table.h"

template <class Aggregator>
class SingleKeyTable : public Table {
 public:
  SingleKeyTable(Table::Key key, Aggregator aggregator,
                 std::unique_ptr<OutputTable> output)
      : key_(std::move(key)),
        aggregator_(std::move(aggregator)),
        output_(std::move(output)) {}

  void PushRow(const InputRow& row) override {
    auto it = state_.find(row[key_.field]);
    if (it == state_.end()) {
      state_.emplace_hint(it, row[key_.field], aggregator_.Init(row));
    } else {
      aggregator_.Update(row, it->second);
    }
  }

  void Finish() override {
    for (const auto& [key, value] : state_) {
      output_->Set(key_.column, key);
      aggregator_.Print(value, *output_);
      output_->EndLine();
    }
    decltype(state_)().swap(state_);
    aggregator_.Reset();
    output_->Finish();
  }

 private:
  absl::flat_hash_map<std::string, typename Aggregator::State> state_;
  Table::Key key_;
  Aggregator aggregator_;
  std::unique_ptr<OutputTable> output_;
};

class SingleKeyNoAggregationTable : public Table {
 public:
  SingleKeyNoAggregationTable(Table::Key key,
                              std::unique_ptr<OutputTable> output)
      : key_(std::move(key)), output_(std::move(output)) {}

  void PushRow(const InputRow& row) override {
    state_.emplace(row[key_.field]);
  }

  void Finish() override {
    for (const auto& key : state_) {
      output_->Set(key_.column, key);
      output_->EndLine();
    }
    decltype(state_)().swap(state_);
    output_->Finish();
  }

 private:
  absl::flat_hash_set<std::string> state_;
  Table::Key key_;
  std::unique_ptr<OutputTable> output_;
};

#endif  // GITHUB_ZISZIS_ZG_SINGLE_KEY_INCLUDED
