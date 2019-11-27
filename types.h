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
  double AsDouble() const;

 private:
  std::string_view value_;
};

template <class T>
T Cast(const FieldValue& field);

template <>
inline int64_t Cast<int64_t>(const FieldValue& field) {
  return field.AsInt64();
}

template <>
inline double Cast<double>(const FieldValue& field) {
  return field.AsDouble();
}

using InputRow = std::vector<FieldValue>;

#endif  // GITHUB_ZISZIS_ZG_TYPES_INCLUDED
