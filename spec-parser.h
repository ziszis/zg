#ifndef GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED
#define GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED

#include <memory>
#include <optional>
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

// Implementation detail of `Parse()`. Only exposed for testing.
class Tokenizer {
 public:
  enum TokenType { END=0, PIPE, ID, OPAREN, CPAREN, COMMA, TILDE };
  struct Token {
    TokenType type;
    std::string_view value;
  };

  explicit Tokenizer(std::string spec);

  Token Peek() { return tokens_[current_]; }
  Token ConsumeId(std::string_view expected_value);
  Token ConsumeAnyId(std::string_view expected_desc);
  Token Consume(TokenType type);
  std::optional<Token> TryConsume(TokenType type);

  [[noreturn]] void FailParse(std::string_view error, int tokens_back = 0);

 private:
  std::string spec_;
  std::vector<Token> tokens_;
  int current_;
};

}  // namespace spec

#endif  // GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED
