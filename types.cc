#include "types.h"

#include "absl/strings/numbers.h"
#include "base.h"

template <>
int64_t ParseAs<>(const FieldValue& field) {
  int64_t result;
  if (!absl::SimpleAtoi(field, &result)) {
    Fail("Failed to parse int64_t: ", Quoted(field));
  }
  return result;
}

template <>
double ParseAs<>(const FieldValue& field) {
  double result;
  if (!absl::SimpleAtod(field, &result)) {
    Fail("Failed to parse double: ", Quoted(field));
  }
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

// Splits line_ into fields_, separated by one or more of tabs/spaces.
void InputRow::SplitLine() const {
  const char* begin = line_.data();
  const char* end = begin + line_.size();
  while (true) {
    while (true) {
      if (begin == end) return;
      if (*begin != ' ' && *begin != '\t') break;
      ++begin;
    }
    const char* p = begin;
    while (true) {
      if (p == end) {
        fields_.emplace_back(std::string_view(begin, p - begin));
        return;
      }
      if (*p == ' ' || *p == '\t') {
        fields_.emplace_back(std::string_view(begin, p - begin));
        begin = p + 1;
        break;
      }
      ++p;
    }
  }
}
