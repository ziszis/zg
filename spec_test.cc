#include "spec.h"

#include "gtest/gtest.h"

namespace spec {
namespace {

TEST(ToStringTest, Smoke) {
  // Date when FOO reached max price:
  Stage max_foo = AggregatedTable{
      .filters = {Filter{.regexp = {.what = Expr{1}, .regexp = "FOO"}}},
      .components = {{Max{.what = {3}, .output = {Expr{2}}}}}};
  EXPECT_EQ(ToString(Pipeline{max_foo}), "filter(_1~FOO) max(_3, _2)");

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
}  // namespace spec
