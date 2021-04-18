#ifndef GITHUB_ZISZIS_ZG_SPEC_INCLUDED
#define GITHUB_ZISZIS_ZG_SPEC_INCLUDED

#include <string>
#include <vector>
#include <variant>

// Parsed form of the pipeline specification. It's produced by the
// spec-parser.h and consumed by pipeline.h to patch together appropriate
// Table/Aggregator objects.

namespace spec {

struct Expr {
  int field;
};

struct Key {
  Expr expr;
};

struct Sum {
  Expr expr;
};

struct Min {
  Expr what;
  std::vector<Expr> output;
};

struct Max {
  Expr what;
  std::vector<Expr> output;
};

struct Count {};

struct CountDistinct {
  Expr what;
};

struct Filter {
  struct RegexpMatch {
    Expr what;
    std::string regexp;
  };
  RegexpMatch regexp;
};

struct AggregatedTable {
  using Component = std::variant<Key, Sum, Min, Max, Count, CountDistinct>;
  std::vector<Component> components;
  std::vector<Filter> filters;
};

struct SimpleTable {
  std::vector<Expr> columns;
  std::vector<Filter> filters;
};

using Stage = std::variant<SimpleTable, AggregatedTable>;
using Pipeline = std::vector<Stage>;

// Converts any of the objects above into its canonical string form (suitable
// for parsing back if necessary). Only used for testing.
template <class T>
std::string ToString(const T& spec_element);

}  // namespace spec

#endif  // GITHUB_ZISZIS_ZG_SPEC_INCLUDED
