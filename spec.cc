#include "spec.h"

#include <regex>

#include "absl/strings/str_cat.h"

namespace spec {
namespace {

struct Delimiters {
  std::string_view leader;
  std::string_view trailer;
  std::string_view delim;
};

template <class T>
std::string ToString(const std::vector<T>& elts, Delimiters d) {
  std::string result;
  for (int i = 0; i < elts.size(); ++i) {
    if (d.leader.data()) result.append(d.leader);
    if (i != 0 && d.delim.data() != nullptr) result.append(d.delim);
    result.append(ToString(elts[i]));
    if (d.trailer.data()) result.append(d.trailer);
  }
  return result;
}

}  // namespace

template <class... T>
std::string ToString(const std::variant<T...>& value) {
  return std::visit([&](const auto& v) { return ToString(v); }, value);
}

template <>
std::string ToString(const Expr& expr) {
  return absl::StrCat("_", expr.field);
}

template <>
std::string ToString(const Key& key) {
  return absl::StrCat("key(", ToString(key.expr), ")");
}

template <>
std::string ToString(const Sum& sum) {
  return absl::StrCat("sum(", ToString(sum.expr), ")");
}

template <>
std::string ToString(const Min& min) {
  return absl::StrCat("min(", ToString(min.what),
                      ToString(min.output, {.leader = ", "}), ")");
}

template <>
std::string ToString(const Max& max) {
  return absl::StrCat("max(", ToString(max.what),
                      ToString(max.output, {.leader = ", "}), ")");
}

template <>
std::string ToString(const Count& count) {
  return "count";
}

template <>
std::string ToString(const CountDistinct& cd) {
  return absl::StrCat("count(distinct, ", ToString(cd.what), ")");
}

template <>
std::string ToString(const Filter& filter) {
  std::string escaped = filter.regexp.regexp;
  if (!std::regex_match(escaped, std::regex("^[a-zA-Z][a-zA-Z0-9]*$"))) {
    escaped = std::regex_replace(escaped, std::regex("\\\\"), "\\\\");
    escaped = std::regex_replace(escaped, std::regex("'"), "\\'");
    escaped = absl::StrCat("'", escaped, "'");
  }
  return absl::StrCat("filter(", ToString(filter.regexp.what), "~", escaped,
                      ")");
}

template <>
std::string ToString(const AggregatedTable& table) {
  return absl::StrCat(ToString(table.filters, {.trailer = " "}),
                      ToString(table.components, {.delim = " "}));
}

template <>
std::string ToString(const SimpleTable& table) {
  return absl::StrCat(
      ToString(table.filters, {.delim = " "}),
      table.columns.empty() ? "" : ToString(table.columns, {.leader = " "}));
}

template <>
std::string ToString(const Pipeline& pipeline) {
  return ToString(pipeline, {.delim=" => "});
}

}  // namespace spec
