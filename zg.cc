#include <cstdlib>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"

void Fail(std::string_view reason) {
  std::cerr << reason << std::endl;
  std::abort();
}

class FieldValue {
 public:
  explicit FieldValue(std::string_view value) : value_(value) {}

  std::string_view AsString() const { return value_; }

  int64_t AsInt64() const {
    if (!int_) {
      int_.emplace();
      if (!absl::SimpleAtoi(value_, &*int_)) {
        Fail("Failed to parse int");
      }
    }
    return *int_;
  }

 private:
  std::string_view value_;
  mutable std::optional<int64_t> int_;
};

void SplitLine(std::string_view line, std::vector<FieldValue>* fields) {
  // Using for-loop instead of the more canonical `fields = absl::StrSplit(...)`
  // to avoid vector reallocations.
  fields->clear();
  for (auto field :
       absl::StrSplit(line, absl::ByAnyChar(" \t"), absl::SkipWhitespace())) {
    fields->emplace_back(field);
  }
}

int main(int argc, char *argv[]) {
  std::string line;
  std::vector<FieldValue> fields;
  while (std::getline(std::cin, line)) {
    SplitLine(line, &fields);
  }
  return 0;
}
