#ifndef GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED
#define GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED

#include <optional>
#include <string>
#include <vector>

#include "spec.h"

namespace spec {

Pipeline Parse(const std::string& spec);

// Implementation detail of `Parse()`. Only exposed for testing.
class Tokenizer {
 public:
  enum TokenType {
    END = 0,
    PIPE,
    ID,
    OPAREN,
    CPAREN,
    COMMA,
    TILDE,
    SQUOTED_STRING,
    DQUOTED_STRING
  };
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
  std::string ConsumeString(std::string_view expected_desc);

  [[noreturn]] void FailParse(std::string_view error, int tokens_back = 0);

 private:
  std::string spec_;
  std::vector<Token> tokens_;
  int current_;
};

}  // namespace spec

#endif  // GITHUB_ZISZIS_ZG_SPEC_PARSER_INCLUDED
