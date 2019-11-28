#ifndef GITHUB_ZISZIS_ZG_NO_AGGREGATION_INCLUDED
#define GITHUB_ZISZIS_ZG_NO_AGGREGATION_INCLUDED

#include <optional>
#include <string_view>
#include <vector>

#include "base.h"
#include "table.h"

std::unique_ptr<Table> MakeNoAggregatorsTable(
    const std::vector<AggregationState::Key>& keys);

#endif  // GITHUB_ZISZIS_ZG_NO_AGGREGATION_INCLUDED
