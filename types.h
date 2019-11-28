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

using InputRow = std::vector<FieldValue>;

#endif  // GITHUB_ZISZIS_ZG_TYPES_INCLUDED
