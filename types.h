#ifndef GITHUB_ZISZIS_ZG_TYPES_INCLUDED
#define GITHUB_ZISZIS_ZG_TYPES_INCLUDED

#include <string_view>
#include <vector>

#include "base.h"

class FieldValue {
 public:
  explicit FieldValue(std::string_view value) : value_(value) {}
  std::string_view AsString() const { return value_; }
  int64_t AsInt64() const;

 private:
  std::string_view value_;
};

using InputRow = std::vector<FieldValue>;

#endif  // GITHUB_ZISZIS_ZG_TYPES_INCLUDED
