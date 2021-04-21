#ifndef GITHUB_ZISZIS_ZG_PIPELINE_INCLUDED
#define GITHUB_ZISZIS_ZG_PIPELINE_INCLUDED

#include <memory>

#include "spec.h"
#include "table.h"

std::unique_ptr<Table> BuildPipeline(spec::Pipeline spec);

#endif  // GITHUB_ZISZIS_ZG_PIPELINE_INCLUDED
