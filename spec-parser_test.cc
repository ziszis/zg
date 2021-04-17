#include "spec-parser.h"

#include "gtest/gtest.h"

namespace spec {
namespace {

TEST(SpecParser, Smoke) {
  Pipeline pipeline = Parse({});
  std::string result = ToString(pipeline);
  EXPECT_EQ(result, "");
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

}  // namespace
}  // spec
