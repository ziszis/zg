#ifndef GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED
#define GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED

#include <memory>
#include <string>
#include <variant>
#include <vector>

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

Pipeline Parse(const std::string& spec);

template <class T>
std::string ToString(const T& spec_element);

std::vector<std::pair<int, std::string_view>> SplitIntoTokens(
    const std::string& s, const std::vector<std::string_view>& token_regexes);

}  // namespace spec

#endif  // GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED
