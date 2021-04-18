#ifndef GITHUB_ZISZIS_ZG_COMPOSITE_KEY_INCLUDED
#define GITHUB_ZISZIS_ZG_COMPOSITE_KEY_INCLUDED

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "table.h"
#include "varint.h"

class BaseCompositeKeyTable : public Table {
 public:
  explicit BaseCompositeKeyTable(std::vector<Table::Key> key)
      : key_(std::move(key)) {}

 protected:
  void SerializeKey(const InputRow& row) {
    buf_.clear();
    for (const auto& key : key_) {
      std::string_view value = row[key.field];
      if (value.size() > std::numeric_limits<uint32_t>::max()) {
        Fail("Key too long, length=", value.size());
      }
      AppendVarint32(value.size(), &buf_);
      buf_.append(value);
    }
  }

  void RenderKey(const std::string& serialized_key, OutputTable& out) const {
    const char* p = serialized_key.data();
    for (const Table::Key& key : key_) {
      uint32_t len = ParseVarint32(p);
      out.Set(key.column, std::string_view(p, len));
      p += len;
    }
  }

  std::vector<Table::Key> key_;
  std::string buf_;
};

template <class Aggregator>
class CompositeKeyTable : public BaseCompositeKeyTable {
 public:
  CompositeKeyTable(std::vector<Table::Key> key, Aggregator aggregator,
                    std::unique_ptr<OutputTable> output)
      : BaseCompositeKeyTable(std::move(key)),
        aggregator_(std::move(aggregator)),
        output_(std::move(output)) {}

  void PushRow(const InputRow& row) override {
    SerializeKey(row);
    auto it = state_.find(buf_);
    if (it == state_.end()) {
      state_.emplace_hint(it, buf_, aggregator_.Init(row));
    } else {
      aggregator_.Update(row, it->second);
    }
  }

  void Finish() override {
    for (const auto& [serialized_key, value] : state_) {
      RenderKey(serialized_key, *output_);
      aggregator_.Print(value, *output_);
      output_->EndLine();
    }
    decltype(state_)().swap(state_);
    output_->Finish();
  }

 private:
  absl::flat_hash_map<std::string, typename Aggregator::State> state_;
  Aggregator aggregator_;
  std::unique_ptr<OutputTable> output_;
};

class CompositeKeyNoAggregationTable : public BaseCompositeKeyTable {
 public:
  CompositeKeyNoAggregationTable(std::vector<Table::Key> key,
                                 std::unique_ptr<OutputTable> output)
      : BaseCompositeKeyTable(std::move(key)), output_(std::move(output)) {}

  void PushRow(const InputRow& row) override {
    SerializeKey(row);
    state_.insert(buf_);
  }

  void Finish() override {
    for (const auto& serialized_key : state_) {
      RenderKey(serialized_key, *output_);
      output_->EndLine();
    }
    decltype(state_)().swap(state_);
    output_->Finish();
  }

 private:
  absl::flat_hash_set<std::string> state_;
  std::unique_ptr<OutputTable> output_;
};

#endif  // GITHUB_ZISZIS_ZG_COMPOSITE_KEY_INCLUDED
