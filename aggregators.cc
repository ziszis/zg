#include "aggregators.h"
#include "absl/strings/str_format.h"

namespace {

template <class T>
bool UpdateMin(T& min, T v) {
  if (v < min) {
    min = v;
    return true;
  } else {
    return false;
  }
}

template <class T>
bool UpdateMax(T& max, T v) {
  if (v > max) {
    max = v;
    return true;
  } else {
    return false;
  }
}

template <class V>
inline bool SumOverflows(V a, V b) {
  return (a > 0 && b > std::numeric_limits<V>::max() - a) ||
         (a < 0 && b > std::numeric_limits<V>::lowest() - a);
}

inline double AsDouble(std::variant<int64_t, double> n) {
  return std::visit([](auto v) { return static_cast<double>(v); }, n);
}

}  // namespace

Numeric Numeric::Make(FieldValue field) {
  std::string_view f = field;
  if (std::any_of(f.begin(), f.end(),
                  [](char c) { return c == '.' || c == 'e' || c == 'E'; })) {
    return Numeric(ParseAs<double>(field));
  } else {
    return Numeric(ParseAs<int64_t>(field));
  }
}

void Numeric::Add(Numeric field) {
  if (int64_t* current = std::get_if<int64_t>(&v_)) {
    if (int64_t* that = std::get_if<int64_t>(&field.v_)) {
      if (!SumOverflows(*current, *that)) {
        *current += *that;
        return;
      }
    }
    v_ = static_cast<double>(*current);
  }
  std::get<double>(v_) += AsDouble(field.v_);
}

bool Numeric::Min(Numeric field) {
  if (int64_t* current = std::get_if<int64_t>(&v_)) {
    if (int64_t* that = std::get_if<int64_t>(&field.v_)) {
      return UpdateMin(*current, *that);
    }
    v_ = static_cast<double>(*current);
  }
  return UpdateMin(std::get<double>(v_), AsDouble(field.v_));
}

bool Numeric::Max(Numeric field) {
  if (int64_t* current = std::get_if<int64_t>(&v_)) {
    if (int64_t* that = std::get_if<int64_t>(&field.v_)) {
      return UpdateMax(*current, *that);
    }
    v_ = static_cast<double>(*current);
  }
  return UpdateMax(std::get<double>(v_), AsDouble(field.v_));
}

void Numeric::Print(std::string* out) const {
  struct {
    void operator()(int64_t v) { absl::StrAppend(out, v); }
    void operator()(double v) { absl::StrAppendFormat(out, "%.8g", v); }
    std::string* out;
  } p{out};
  std::visit(p, v_);
}
