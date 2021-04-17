#include "spec-parser.h"

#include <optional>
#include <regex>

#include "absl/strings/str_cat.h"
#include "base.h"

namespace spec {

namespace {

class Parser {
 public:
  explicit Parser(std::string spec) : spec_(std::move(spec)), current_(0) {
    std::vector<std::string_view> regexes = {
        "[ \t]+",                  // END / WHITESPACE
        "=>",                      // PIPE
        "[_a-zA-Z][_a-zA-Z0-9]*",  // ID
        "\\(",                     // OPAREN
        "\\)",                     // CPAREN
        ",",                       // COMMA
        "~",                       // TILDE
    };
    for (auto [type, value] : SplitIntoTokens(spec_, regexes)) {
      if (type == 0) continue;
      tokens_.push_back(Token{static_cast<TokenType>(type), value});
    }
    tokens_.push_back({END, std::string_view(spec_.data() + spec_.size())});
  }

  Pipeline ParsePipeline() {
    Pipeline pipeline;
    while (true) {
      pipeline.push_back(ParseStage());
      if (Peek().type == END) return pipeline;
      Consume(PIPE);
    }
  }

  Stage ParseStage() {
    std::vector<AggregatedTable::Component> agg;
    std::vector<Filter> filters;

    while (true) {
      Token token = Peek();
      if (token.type == END || token.type == PIPE) break;

      if (token.value == "key") {
        ConsumeId("key");
        Consume(OPAREN);
        agg.push_back(Key{ParseExpr()});
        Consume(CPAREN);
      } else if (token.value == "sum") {
        ConsumeId("sum");
        Consume(OPAREN);
        agg.push_back(Sum{ParseExpr()});
        Consume(CPAREN);
      } else if (token.value == "min") {
        agg.push_back(ParseMin());
      } else if (token.value == "max") {
        agg.push_back(ParseMax());
      } else if (token.value == "count") {
        agg.push_back(ParseCount());
      } else if (token.value == "c") {
        ConsumeId("c");
        agg.push_back(Count{});
      } else if (token.value == "filter") {
        filters.push_back(ParseFilter());
      } else if (auto expr = TryShortForm("k")) {
        agg.push_back(Key{*expr});
      } else if (auto expr = TryShortForm("s")) {
        agg.push_back(Sum{*expr});
      } else if (auto expr = TryShortForm("m")) {
        agg.push_back(Min{*expr});
      } else if (auto expr = TryShortForm("M")) {
        agg.push_back(Max{*expr});
      } else if (auto expr = TryShortForm("cd")) {
        agg.push_back(CountDistinct{*expr});
      } else if (auto expr = TryShortForm("f")) {
        filters.push_back(ParseShortFilter(*expr));
      } else if (token.value == "f") {
        ConsumeId("f");
        filters.push_back(ParseShortFilter(Expr{.field = 0}));
      } else {
        FailParse("unexpected token");
      }
    }

    if (agg.empty()) {
      return SimpleTable{.columns = {Expr{.field = 0}}, .filters = filters};
    } else {
      return AggregatedTable{.components = agg, .filters = filters};
    }
  }

  Expr ParseExpr() {
    std::string_view column = ConsumeAnyId("column reference").value;
    if (!column.starts_with('_')) {
      FailParse("expected column reference", 1);
    }
    column.remove_prefix(1);
    int index = 0;
    if (!absl::SimpleAtoi(column, &index)) {
      FailParse("cannot parse column index", 1);
    }
    return Expr{index};
  }

  Min ParseMin() {
    Min result;
    ConsumeId("min");
    Consume(OPAREN);
    result.what = ParseExpr();
    while (TryConsume(COMMA)) {
      result.output.push_back(ParseExpr());
    }
    Consume(CPAREN);
    return result;
  }

  Max ParseMax() {
    Max result;
    ConsumeId("max");
    Consume(OPAREN);
    result.what = ParseExpr();
    while (TryConsume(COMMA)) {
      result.output.push_back(ParseExpr());
    }
    Consume(CPAREN);
    return result;
  }

  AggregatedTable::Component ParseCount() {
    ConsumeId("count");
    if (TryConsume(OPAREN)) {
      ConsumeId("distinct");
      Consume(COMMA);
      Expr what = ParseExpr();
      Consume(CPAREN);
      return CountDistinct{what};
    } else {
      return Count{};
    }
  }

  Filter ParseFilter() {
    ConsumeId("filter");
    Consume(OPAREN);
    Expr what = ParseExpr();
    Consume(TILDE);
    std::string regexp = ParseString("regexp");
    Consume(CPAREN);
    return Filter{.regexp{what, regexp}};
  }

  Filter ParseShortFilter(Expr what) {
    Consume(TILDE);
    std::string regexp = ParseString("regexp");
    return Filter{.regexp{what, regexp}};
  }

  std::optional<Expr> TryShortForm(std::string_view shortform) {
    Token token = Peek();
    if (token.type != ID || !token.value.starts_with(shortform) ||
        token.value == shortform) {
      return std::nullopt;
    }
    Consume(ID);
    int index = 0;
    if (absl::SimpleAtoi(token.value.substr(shortform.size()), &index)) {
      return Expr{.field = index};
    } else {
      return std::nullopt;
    }
  }

 private:
  enum TokenType { END=0, PIPE, ID, OPAREN, CPAREN, COMMA, TILDE };
  struct Token {
    TokenType type;
    std::string_view value;
  };

  Token Peek() { return tokens_[current_]; }

  Token ConsumeAnyId(std::string_view expected) {
    Token token = Peek();
    if (token.type != ID) {
      FailParse(absl::StrCat("expected ", expected));
    }
    return tokens_[current_++];
  }

  Token ConsumeId(std::string_view expected) {
    Token token = Peek();
    if (token.type != ID || token.value != expected) {
      FailParse(absl::StrCat("expected '", expected, "'"));
    }
    return tokens_[current_++];
  }

  Token Consume(TokenType type) {
    if (type != Peek().type) {
      std::string_view expected = [&] {
        switch (type) {
          case END:
            return "end of spec";
          case PIPE:
            return "'=>'";
          case ID:
            return "???";
          case OPAREN:
            return "'('";
          case CPAREN:
            return "')'";
          case COMMA:
            return "','";
          case TILDE:
            return "'~'";
        }
      }();
      FailParse(absl::StrCat("expected ", expected));
    }
    return tokens_[current_++];
  }

  std::optional<Token> TryConsume(TokenType type) {
    if (tokens_[current_].type == type) {
      return tokens_[current_++];
    } else {
      return std::nullopt;
    }
  }

  [[noreturn]] void FailParse(std::string_view error, int tokens_back = 0) {
    Token failure = tokens_[current_ - tokens_back];
    Fail("Failed to parse spec: ", error, "\n", spec_, "\n",
         std::string(failure.value.data() - spec_.data(), ' '), "^");
  }

  std::string ParseString(std::string_view expected) {
    return std::string(ConsumeAnyId(expected).value);
  }

  std::string spec_;
  std::vector<Token> tokens_;
  int current_;
};

}  // namespace

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

Pipeline Parse(const std::string& spec) { return Parser(spec).ParsePipeline(); }

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
