#include "spec-parser.h"

#include <regex>

#include "absl/strings/str_cat.h"
#include "base.h"

namespace spec {

template <class T>
std::string ToString(const T&);

template <class... T>
std::string ToString(const std::variant<T...>& value) {
  return std::visit([&](const auto& v) { return ToString(v); }, value);
}

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
  return absl::StrCat("filter(", ToString(filter.regexp.what), "~",
                      filter.regexp.regexp, ")");
}

template <>
std::string ToString(const AggregatedTable& table) {
  return absl::StrCat(ToString(table.filters, {.trailer = " "}),
                      ToString(table.components, {.delim = " "}));
}

template <>
std::string ToString(const SimpleTable& table) {
  return absl::StrCat(ToString(table.filters, {.trailer = " "}),
                      ToString(table.columns, {.delim = " "}));
}

Pipeline Parse(const std::vector<std::string>& spec) { return {}; }

template <>
std::string ToString(const Pipeline& pipeline) {
  return ToString(pipeline, {.delim=" => "});
}

std::vector<std::pair<int, std::string_view>> SplitIntoTokens(
    const std::string& s, const std::vector<std::string_view>& token_regexes) {
  std::string composed_regex;
  for (const auto& r : token_regexes) {
    if (!composed_regex.empty()) composed_regex.push_back('|');
    composed_regex.append("^(");
    composed_regex.append(r);
    composed_regex.push_back(')');
  }
  std::regex regex(composed_regex);

  std::vector<std::pair<int, std::string_view>> result;
  std::cmatch matches;
  const char* begin = s.data();
  const char* end = begin + s.size();
  while (begin != end) {
    if (!std::regex_search<const char*>(begin, end, matches, regex)) {
      Fail(absl::StrCat("Unrecognized token\n", s, "\n",
                        std::string(begin - s.data(), ' '), "^"));
    }
    for (int i = 0; i < token_regexes.size(); ++i) {
      const auto& match = matches[i + 1];
      if (match.matched) {
        result.push_back(
            {i, std::string_view(match.first, match.second - match.first)});
        begin = match.second;
        break;
      }
    }
  }
  return result;
}

}  // namespace spec
