#ifndef GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED
#define GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED

#include <memory>
#include <string>
#include <vector>

#include "table.h"

class OutputTable {
 public:
  explicit OutputTable(int num_columns) : columns_(num_columns) {}
  virtual ~OutputTable() {}

  void Set(int column, std::string_view value) { columns_[column] = value; }
  virtual void EndLine() = 0;
  virtual void Finish() = 0;

 protected:
  std::vector<std::string_view> columns_;
};

std::unique_ptr<OutputTable> MakeStdoutTable(int num_columns);

std::unique_ptr<OutputTable> MakePipeTable(int num_columns,
                                           std::unique_ptr<Table> table);

std::unique_ptr<Table> MakePassthroughTable();

#endif  // GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED
