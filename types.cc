#include "types.h"

#include "absl/strings/numbers.h"
#include "base.h"

int64_t FieldValue::AsInt64() const {
  int64_t result;
  if (!absl::SimpleAtoi(value_, &result)) Fail("Failed to parse int");
  return result;
}
