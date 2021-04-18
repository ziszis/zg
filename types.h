#ifndef GITHUB_ZISZIS_ZG_TYPES_INCLUDED
#define GITHUB_ZISZIS_ZG_TYPES_INCLUDED

#include <optional>
#include <string_view>
#include <vector>

#include "base.h"

class FieldValue {
 public:
  explicit FieldValue(std::string_view value) : value_(value) {}
  operator std::string_view() const { return value_; }

 private:
  std::string_view value_;
};

template <class T, class... Ts>
constexpr bool is_one_of() {
  return (... || std::is_same<T, Ts>::value);
}

template <class T, std::enable_if_t<is_one_of<T, int64_t, double>(), int> = 0>
T ParseAs(const FieldValue&);

template <class T, std::enable_if_t<is_one_of<T, int64_t>(), int> = 0>
std::optional<T> TryParseAs(const FieldValue&);

class InputRow {
 public:
  inline void Reset(std::string_view line) {
    line_ = line;
    fields_.clear();
  }

  void Reset(const std::vector<std::string_view>& columns) {
    fields_.clear();
    line_ = std::string_view();
    for (auto c : columns) fields_.push_back(FieldValue(c));
  }

  FieldValue operator[](int i) const {
    if (i == 0) {
      if (line_.data() == nullptr) BuildLine();
      return FieldValue(line_);
    }
    if (fields_.empty()) SplitLine();
    return fields_[i - 1];
  }

 private:
  void SplitLine() const;
  void BuildLine() const;

  mutable std::string_view line_;
  mutable std::vector<FieldValue> fields_;
  mutable std::string line_buf_;
};

#endif  // GITHUB_ZISZIS_ZG_TYPES_INCLUDED
