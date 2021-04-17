#include "spec-parser.h"

#include "gtest/gtest.h"

namespace spec {
namespace {

TEST(SpecParser, Empty) {
  EXPECT_EQ(ToString(Parse("")), "_0");
}

TEST(SpecParser, Smoke) {
  EXPECT_EQ(ToString(Parse({"count key(_1) => count"})),
            "count key(_1) => count");
}

TEST(SpecParser, MoreSpaces) {
  EXPECT_EQ(ToString(Parse({"count( distinct,_1)  "})), "count(distinct, _1)");
}

TEST(SpecParser, ShortForms1) {
  EXPECT_EQ(ToString(Parse({"c"})), "count");
}

TEST(SpecParser, ShortForms2) {
  EXPECT_EQ(ToString(Parse({"c k1=>c"})), "count key(_1) => count");
}

TEST(SpecParser, ShortForms3) {
  EXPECT_EQ(ToString(Parse({"f~AAPL c"})), "filter(_0~AAPL) count");
  EXPECT_EQ(ToString(Parse({" f1~ AAPL"})), "filter(_1~AAPL) _0");
  EXPECT_EQ(ToString(Parse({"f2 ~AAPL "})), "filter(_2~AAPL) _0");
}

TEST(SpecParser, ToString) {
  // Date when AAPL reached max price:
  Stage max_aapl = AggregatedTable{
      .filters = {Filter{.regexp = {.what = Expr{1}, .regexp = "AAPL"}}},
      .components = {{Max{.what = {3}, .output = {Expr{2}}}}}};
  EXPECT_EQ(ToString(Pipeline{max_aapl}), "filter(_1~AAPL) max(_3, _2)");

  // Number of unique tickers:
  Pipeline num_tickers = {
      Stage{AggregatedTable{.components = {{Key{Expr{1}}}}}},
      Stage{AggregatedTable{.components = {{Count{}}}}}};
  EXPECT_EQ(ToString(num_tickers), "key(_1) => count");

  // Alternative for the same
  num_tickers = {
      Stage{AggregatedTable{.components = {{CountDistinct{Expr{1}}}}}}};
  EXPECT_EQ(ToString(num_tickers), "count(distinct, _1)");
}

TEST(SpecParser, Tmp) {
  enum TokenType { END, PIPE, ID, OPAREN, CPAREN, COMMA, TILDE };
  std::vector<std::string_view> regexes = {
      "[ \t]+", "=>", "[_a-zA-Z][_a-zA-Z0-9]+", "\\(", "\\)", ",", "~",
  };

  using Expected = std::vector<std::pair<int, std::string_view>>;
  Expected expected = {
    {2, "filter"},
    {3, "("},
    {2, "_2"},
    {6, "~"},
    {2, "AAPL"},
    {4, ")"},
    {0, " "},
    {1, "=>"},
    {0, " "},
    {2, "count"},
    {3, "("},
    {2, "distinct"},
    {5, ","},
    {0, " "},
    {2, "_1"},
    {4, ")"},
    {0, " "},
  };
  EXPECT_EQ(SplitIntoTokens("filter(_2~AAPL) => count(distinct, _1) ", regexes),
            expected);
}

}  // namespace
}  // spec
