#include "simple-table.h"

namespace {

class SimpleTable : public Table {
 public:
  SimpleTable(const std::vector<spec::Expr>& columns,
              std::unique_ptr<OutputTable> output)
      : output_(std::move(output)) {
    for (const auto& expr : columns) {
      columns_.push_back(expr.field);
    }
  }

  void PushRow(const InputRow& row) override {
    for (int i = 0; i < columns_.size(); ++i) {
      output_->Set(i, row[columns_[i]]);
    }
    output_->EndLine();
  }

  void Finish() override { output_->Finish(); }

 private:
  std::vector<int> columns_;
  std::unique_ptr<OutputTable> output_;
};

}  // namespace

std::unique_ptr<Table> MakeSimpleTable(const std::vector<spec::Expr>& columns,
                                       std::unique_ptr<OutputTable> output) {
  return std::make_unique<SimpleTable>(columns, std::move(output));
}
