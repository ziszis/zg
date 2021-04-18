#ifndef GITHUB_ZISZIS_ZG_FILTER_TABLE_INCLUDED
#define GITHUB_ZISZIS_ZG_FILTER_TABLE_INCLUDED

#include <memory>
#include <vector>

#include "spec.h"
#include "table.h"

std::unique_ptr<Table> WrapFilter(const std::vector<spec::Filter>& filters,
                                  std::unique_ptr<Table> table);

#endif  // GITHUB_ZISZIS_ZG_FILTER_TABLE_INCLUDED
