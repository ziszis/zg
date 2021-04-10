#ifndef GITHUB_ZISZIS_ZG_SINGLE_KEY_INCLUDED
#define GITHUB_ZISZIS_ZG_SINGLE_KEY_INCLUDED

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "table.h"

template <class Aggregator>
class SingleKeyTable : public Table {
 public:
  SingleKeyTable(Table::Key key, Aggregator aggregator)
      : key_(std::move(key)), aggregator_(std::move(aggregator)) {}

  void PushRow(const InputRow& row) override {
    auto it = state_.find(row[key_.field]);
    if (it == state_.end()) {
      state_.emplace_hint(it, row[key_.field], aggregator_.Init(row));
    } else {
      aggregator_.Update(row, it->second);
    }
  }

  void Render(OutputBuffer* out) const override {
    for (const auto& [key, value] : state_) {
      out->Column(key_.column)->assign(key);
      aggregator_.Print(value, out);
      out->EndLine();
    }
  }

 private:
  absl::flat_hash_map<std::string, typename Aggregator::State> state_;
  Table::Key key_;
  Aggregator aggregator_;
};

class SingleKeyNoAggregationTable : public Table {
 public:
  explicit SingleKeyNoAggregationTable(Table::Key key) : key_(std::move(key)) {}

  void PushRow(const InputRow& row) override {
    state_.emplace(row[key_.field]);
  }

  void Render(OutputBuffer* out) const override {
    for (const auto& key : state_) {
      out->Column(key_.column)->assign(key);
      out->EndLine();
    }
  }

 private:
  absl::flat_hash_set<std::string> state_;
  Table::Key key_;
};

#endif  // GITHUB_ZISZIS_ZG_SINGLE_KEY_INCLUDED
