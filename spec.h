#ifndef GITHUB_ZISZIS_ZG_SPEC_INCLUDED
#define GITHUB_ZISZIS_ZG_SPEC_INCLUDED

#include <memory>
#include <string>
#include <vector>

#include "table.h"

std::unique_ptr<Table> ParseSpec(const std::vector<std::string>& spec);

#endif  // GITHUB_ZISZIS_ZG_SPEC_INCLUDED
