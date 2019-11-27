#include "types.h"

#include "absl/strings/numbers.h"
#include "base.h"

int64_t FieldValue::AsInt64() const {
  if (auto parsed = TryAsInt64()) {
    return *parsed;
  } else {
    Fail("Failed to parse int");
    return 0;
  }
}

std::optional<int64_t> FieldValue::TryAsInt64() const {
  int64_t result;
  if (absl::SimpleAtoi(value_, &result)) {
    return result;
  } else {
    return std::nullopt;
  }
}

double FieldValue::AsDouble() const {
  double result;
  if (!absl::SimpleAtod(value_, &result)) Fail("Failed to parse double");
  return result;
}
