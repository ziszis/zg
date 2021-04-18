#include "filter-table.h"

#include <utility>

#include "re2/re2.h"

namespace {

using re2::RE2;

class FilterTable : public Table {
 public:
  FilterTable(const std::vector<spec::Filter>& filters,
              std::unique_ptr<Table> output)
      : output_(std::move(output)) {
    for (const auto& spec : filters) {
      filters_.emplace_back(spec.regexp.what.field,
                            std::make_unique<RE2>(spec.regexp.regexp));
    }
  }

  void PushRow(const InputRow& row) override {
    for (const auto& f : filters_) {
      std::string_view field = row[f.first];
      if (!re2::RE2::PartialMatch(field, *f.second)) return;
    }
    output_->PushRow(row);
  }

  void Finish() override { output_->Finish(); }

 private:
  std::vector<std::pair<int, std::unique_ptr<RE2>>> filters_;
  std::unique_ptr<Table> output_;
};

}  // namespace

std::unique_ptr<Table> WrapFilter(const std::vector<spec::Filter>& filters,
                                  std::unique_ptr<Table> output) {
  if (filters.empty()) {
    return output;
  } else {
    return std::make_unique<FilterTable>(filters, std::move(output));
  }
}
