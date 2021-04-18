#ifndef GITHUB_ZISZIS_ZG_SIMPLE_TABLE_INCLUDED
#define GITHUB_ZISZIS_ZG_SIMPLE_TABLE_INCLUDED

#include <memory>
#include <vector>

#include "output.h"
#include "spec.h"
#include "table.h"

std::unique_ptr<Table> MakeSimpleTable(const std::vector<spec::Expr>& columns,
                                       std::unique_ptr<OutputTable> output);

#endif  // GITHUB_ZISZIS_ZG_SIMPLE_TABLE_INCLUDED
