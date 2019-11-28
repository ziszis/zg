#include "no-aggregation.h"

#include "absl/container/flat_hash_set.h"

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

}  // namespace

std::unique_ptr<Table> MakeNoAggregatorsTable(
    const std::vector<AggregationState::Key>& keys) {
  if (keys.size() == 0) {
    Fail("Nothing to do");
    return nullptr;
  } else if (keys.size() == 1) {
    if (keys[0].column != 0) {
      Fail("Should never happen (tm)");
      return nullptr;
    }
    return std::make_unique<NoAggregatorsOneKeyTable>(keys[0].field);
  } else {
    Fail("Composite keys not supported yet");
    return nullptr;
  }
}
