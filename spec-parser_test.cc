#include "spec-parser.h"

#include "gtest/gtest.h"

namespace spec {
namespace {

TEST(SpecParser, Smoke) {
  Pipeline pipeline = Parse({});
  std::string result = Print(pipeline);
  EXPECT_EQ(result, "");
}

}  // namespace
}  // spec
