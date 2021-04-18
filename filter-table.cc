#include "filter-table.h"

#include <regex>
#include <utility>

namespace {

class FilterTable : public Table {
 public:
  FilterTable(const std::vector<spec::Filter>& filters,
              std::unique_ptr<Table> output)
      : output_(std::move(output)) {
    for (const auto& spec : filters) {
      filters_.emplace_back(
          spec.regexp.what.field,
          std::regex(spec.regexp.regexp, std::regex::optimize));
    }
  }

  void PushRow(const InputRow& row) override {
    for (const auto& f : filters_) {
      std::string_view field = row[f.first];
      if (!std::regex_search(field.begin(), field.end(), f.second)) {
        return;
      }
    }
    output_->PushRow(row);
  }

  void Finish() override { output_->Finish(); }

 private:
  std::vector<std::pair<int, std::regex>> filters_;
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
