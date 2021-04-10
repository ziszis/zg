#include "no-aggregation.h"

#include "absl/container/flat_hash_set.h"
#include "varint.h"

namespace {

class NoAggregatorsOneKeyTable : public Table {
 public:
  explicit NoAggregatorsOneKeyTable(int key) : key_(key) {}

  void PushRow(const InputRow& row) override {
    data_.insert(std::string(row[key_]));
  }

  void Render() override {
    OutputBuffer out;
    for (const auto& k : data_) {
      out.Column(0)->assign(k);
      out.EndLine();
    }
  }

 private:
  absl::flat_hash_set<std::string> data_;
  int key_;
};

class NoAggregatorsMultiKeyTable : public Table {
 public:
  explicit NoAggregatorsMultiKeyTable(std::vector<AggregationState::Key> key)
      : key_(std::move(key)) {}

  void PushRow(const InputRow& row) override {
    buf_.clear();
    for (const auto& key : key_) {
      std::string_view value = row[key.field];
      if (value.size() > std::numeric_limits<uint32_t>::max()) {
        Fail("Key too long, length=", value.size());
      }
      AppendVarint32(value.size(), &buf_);
      buf_.append(value);
    }
    data_.insert(buf_);
  }

  void Render() override {
    OutputBuffer out;
    for (const auto& k : data_) {
      const char* p = k.data();
      for (const auto& key : key_) {
        uint32_t len = ParseVarint32(p);
        out.Column(key.column)->assign(p, len);
        p += len;
      }
      out.EndLine();
    }
  }

 private:
  absl::flat_hash_set<std::string> data_;
  std::vector<AggregationState::Key> key_;
  std::string buf_;
};

}  // namespace

std::unique_ptr<Table> MakeNoAggregatorsTable(
    const std::vector<AggregationState::Key>& keys) {
  if (keys.size() == 0) {
    Fail("Nothing to do");
  } else if (keys.size() == 1) {
    if (keys[0].column != 0) Fail("Should never happen (tm)");
    return std::make_unique<NoAggregatorsOneKeyTable>(keys[0].field);
  } else {
    return std::make_unique<NoAggregatorsMultiKeyTable>(keys);
  }
}
