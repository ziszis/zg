#include "spec-parser.h"

#include "gtest/gtest.h"

namespace spec {
namespace {

TEST(SpecParserTest, Empty) {
  EXPECT_EQ(ToString(Parse("")), "");
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
  EXPECT_EQ(ToString(Parse("m2_3")), "min(_2, _3)");
  EXPECT_EQ(ToString(Parse("m2_3_2")), "min(_2, _3, _2)");
}

TEST(SpecParserTest, Max) {
  EXPECT_EQ(ToString(Parse("max(_1)")), "max(_1)");
  EXPECT_EQ(ToString(Parse("M2")), "max(_2)");
  EXPECT_EQ(ToString(Parse("max(_1, _2, _3)")), "max(_1, _2, _3)");
  EXPECT_EQ(ToString(Parse("M2_3")), "max(_2, _3)");
  EXPECT_EQ(ToString(Parse("M2_3_2")), "max(_2, _3, _2)");
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
  EXPECT_EQ(ToString(Parse("filter(_1~FOO)")), "filter(_1~FOO)");
  EXPECT_EQ(ToString(Parse("f~FOO")), "filter(_0~FOO)");
  EXPECT_EQ(ToString(Parse(" f1~ FOO")), "filter(_1~FOO)");
  EXPECT_EQ(ToString(Parse("f2 ~FOO ")), "filter(_2~FOO)");
  EXPECT_EQ(ToString(Parse("f~'FOO'")), "filter(_0~FOO)");
  EXPECT_EQ(ToString(Parse("f~'.*\\\\.US'")), "filter(_0~'.*\\\\.US')");
}

TEST(SpecParserTest, MoreSpaces) {
  EXPECT_EQ(ToString(Parse(" count( distinct,_1 )  ")), "count(distinct, _1)");
}

TEST(SpecParserTest, Pipeline) {
  EXPECT_EQ(ToString(Parse("c=>c=>c")), "count => count => count");
}

TEST(TokenizerTest, Smoke) {
  Tokenizer tokenizer("filter(_2~FOO) => count(distinct, _1) ");
  tokenizer.ConsumeId("filter");
  tokenizer.Consume(Tokenizer::OPAREN);
  tokenizer.ConsumeId("_2");
  tokenizer.Consume(Tokenizer::TILDE);
  tokenizer.ConsumeId("FOO");
  tokenizer.Consume(Tokenizer::CPAREN);
  tokenizer.Consume(Tokenizer::PIPE);
  tokenizer.ConsumeId("count");
  tokenizer.Consume(Tokenizer::OPAREN);
  tokenizer.ConsumeId("distinct");
  tokenizer.Consume(Tokenizer::COMMA);
  tokenizer.ConsumeId("_1");
  tokenizer.Consume(Tokenizer::CPAREN);
}

TEST(TokenizerTest, Strings) {
  Tokenizer tokenizer(R"END(
    abc d1
    'abc' '' '\\' '"\''
    "" "'" "\\\\'\""
  )END");
  EXPECT_EQ(tokenizer.ConsumeString(""), "abc");
  EXPECT_EQ(tokenizer.ConsumeString(""), "d1");
  EXPECT_EQ(tokenizer.ConsumeString(""), "abc");
  EXPECT_EQ(tokenizer.ConsumeString(""), "");
  EXPECT_EQ(tokenizer.ConsumeString(""), "\\");
  EXPECT_EQ(tokenizer.ConsumeString(""), "\"'");
  EXPECT_EQ(tokenizer.ConsumeString(""), "");
  EXPECT_EQ(tokenizer.ConsumeString(""), "'");
  EXPECT_EQ(tokenizer.ConsumeString(""), "\\\\'\"");
}

}  // namespace
}  // namespace spec
