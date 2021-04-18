#include "output.h"

#include <cstdio>

#include "base.h"

namespace {

class StdoutOutputTable : public OutputTable {
 public:
  explicit StdoutOutputTable(int num_columns) : OutputTable(num_columns) {}

  ~StdoutOutputTable() {
    if (!buf_.empty()) LogicError("unfinished table");
  }

  void EndLine() override {
    for (auto c : columns_) {
      buf_.append(c);
      buf_.push_back('\t');
    }
    buf_.back() = '\n';
    if (buf_.size() > 1 << 15) Flush();
  }

  void Finish() override { Flush(); }

  void Flush() {
    if (std::fwrite(buf_.data(), 1, buf_.size(), stdout) != buf_.size()) {
      Fail("Write failed");
    }
    buf_.clear();
  }

 private:
  std::string buf_;
};

class PipeOutputTable : public OutputTable {
 public:
  PipeOutputTable(int num_columns, std::unique_ptr<Table> table)
      : OutputTable(num_columns), table_(std::move(table)) {}

  void EndLine() override {
    row_.Reset(columns_);
    table_->PushRow(row_);
  }

  void Finish() override { table_->Finish(); }

 private:
  std::unique_ptr<Table> table_;
  InputRow row_;
};

}  // namespace

std::unique_ptr<OutputTable> MakeStdoutTable(int num_columns) {
  return std::make_unique<StdoutOutputTable>(num_columns);
}

std::unique_ptr<OutputTable> MakePipeTable(int num_columns,
                                           std::unique_ptr<Table> table) {
  return std::make_unique<PipeOutputTable>(num_columns, std::move(table));
}
