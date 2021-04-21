#include "output.h"

#include <cstdio>

#include "base.h"

namespace {

class BufferedStdout {
 public:
  ~BufferedStdout() {
    if (!buf_.empty()) LogicError("unfinished table");
  }

  std::string& buf() { return buf_; }

  void Flush() {
    if (std::fwrite(buf_.data(), 1, buf_.size(), stdout) != buf_.size()) {
      Fail("Write failed");
    }
    buf_.clear();
  }

 private:
  std::string buf_;
};

class StdoutOutputTable : public OutputTable {
 public:
  explicit StdoutOutputTable(int num_columns) : OutputTable(num_columns) {}

  void EndLine() override {
    std::string& buf = buf_.buf();
    for (auto c : columns_) {
      buf.append(c);
      buf.push_back('\t');
    }
    buf.back() = '\n';
    if (buf.size() > 1 << 15) buf_.Flush();
  }

  void Finish() override { buf_.Flush(); }

 private:
  BufferedStdout buf_;
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

class PassthroughTable : public Table {
 public:
  void PushRow(const InputRow& row) override {
    buf_.buf().append(row[0]);
    buf_.buf().push_back('\n');
  }

  void Finish() override { buf_.Flush(); }

 private:
  BufferedStdout buf_;
};

}  // namespace

std::unique_ptr<OutputTable> MakeStdoutTable(int num_columns) {
  return std::make_unique<StdoutOutputTable>(num_columns);
}

std::unique_ptr<OutputTable> MakePipeTable(int num_columns,
                                           std::unique_ptr<Table> table) {
  return std::make_unique<PipeOutputTable>(num_columns, std::move(table));
}

std::unique_ptr<Table> MakePassthroughTable() {
  return std::make_unique<PassthroughTable>();
}
