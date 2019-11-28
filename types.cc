#include "types.h"

#include "absl/strings/numbers.h"
#include "base.h"

template <>
int64_t ParseAs<>(const FieldValue& field) {
  int64_t result;
  if (!absl::SimpleAtoi(field, &result)) Fail("Failed to parse int64_t");
  return result;
}

template <>
double ParseAs<>(const FieldValue& field) {
  double result;
  if (!absl::SimpleAtod(field, &result)) Fail("Failed to parse double");
  return result;
}

template <>
std::optional<int64_t> TryParseAs<>(const FieldValue& field) {
  int64_t result;
  if (absl::SimpleAtoi(field, &result)) {
    return result;
  } else {
    return std::nullopt;
  }
}
