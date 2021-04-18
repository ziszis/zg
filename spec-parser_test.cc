#include "spec-parser.h"

#include "gtest/gtest.h"

namespace spec {
namespace {

TEST(SpecParserTest, Empty) {
  EXPECT_EQ(ToString(Parse("")), "_0");
}

TEST(SpecParserTest, Smoke) {
  EXPECT_EQ(ToString(Parse("count key(_1) => count")),
            "count key(_1) => count");
}

TEST(SpecParserTest, Sum) {
  EXPECT_EQ(ToString(Parse("sum( _1 )")), "sum(_1)");
  EXPECT_EQ(ToString(Parse("s2")), "sum(_2)");
}

TEST(SpecParserTest, Min) {
  EXPECT_EQ(ToString(Parse("min(_1)")), "min(_1)");
  EXPECT_EQ(ToString(Parse("m2")), "min(_2)");
  EXPECT_EQ(ToString(Parse("min(_1, _2, _3)")), "min(_1, _2, _3)");
}

TEST(SpecParserTest, Max) {
  EXPECT_EQ(ToString(Parse("max(_1)")), "max(_1)");
  EXPECT_EQ(ToString(Parse("M2")), "max(_2)");
  EXPECT_EQ(ToString(Parse("max(_1, _2, _3)")), "max(_1, _2, _3)");
}

TEST(SpecParserTest, Count) {
  EXPECT_EQ(ToString(Parse(" count ")), "count");
  EXPECT_EQ(ToString(Parse("c")), "count");
}

TEST(SpecParserTest, CountDistinct) {
  EXPECT_EQ(ToString(Parse("count(distinct, _1)")), "count(distinct, _1)");
  EXPECT_EQ(ToString(Parse("cd2")), "count(distinct, _2)");
}

TEST(SpecParserTest, Filter) {
  EXPECT_EQ(ToString(Parse("filter(_1~AAPL)")), "filter(_1~AAPL) _0");
  EXPECT_EQ(ToString(Parse("f~AAPL")), "filter(_0~AAPL) _0");
  EXPECT_EQ(ToString(Parse(" f1~ AAPL")), "filter(_1~AAPL) _0");
  EXPECT_EQ(ToString(Parse("f2 ~AAPL ")), "filter(_2~AAPL) _0");
}

TEST(SpecParserTest, MoreSpaces) {
  EXPECT_EQ(ToString(Parse(" count( distinct,_1 )  ")), "count(distinct, _1)");
}

TEST(SpecParserTest, Pipeline) {
  EXPECT_EQ(ToString(Parse("c=>c=>c")), "count => count => count");
}

TEST(SpecParserTest, ToString) {
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

TEST(TokenizerTest, Smoke) {
  Tokenizer tokenizer("filter(_2~AAPL) => count(distinct, _1) ");
  tokenizer.ConsumeId("filter");
  tokenizer.Consume(Tokenizer::OPAREN);
  tokenizer.ConsumeId("_2");
  tokenizer.Consume(Tokenizer::TILDE);
  tokenizer.ConsumeId("AAPL");
  tokenizer.Consume(Tokenizer::CPAREN);
  tokenizer.Consume(Tokenizer::PIPE);
  tokenizer.ConsumeId("count");
  tokenizer.Consume(Tokenizer::OPAREN);
  tokenizer.ConsumeId("distinct");
  tokenizer.Consume(Tokenizer::COMMA);
  tokenizer.ConsumeId("_1");
  tokenizer.Consume(Tokenizer::CPAREN);
}

}  // namespace
}  // namespace spec
