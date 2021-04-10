#include "aggregators.h"
#include "absl/strings/str_format.h"

namespace {

template <class V>
inline bool SumOverflows(V a, V b) {
  return (a > 0 && b > std::numeric_limits<V>::max() - a) ||
         (a < 0 && b > std::numeric_limits<V>::lowest() - a);
}

}  // namespace

void Numeric::Add(const FieldValue& field) {
  if (std::holds_alternative<std::monostate>(v_)) v_ = int64_t{0};
  if (std::holds_alternative<int64_t>(v_)) {
    if (std::optional<int64_t> that = TryParseAs<int64_t>(field)) {
      int64_t& current = std::get<int64_t>(v_);
      if (!SumOverflows(current, *that)) {
        current += *that;
        return;
      }
    }
    v_ = static_cast<double>(std::get<int64_t>(v_));
  }
  std::get<double>(v_) += ParseAs<double>(field);
}

void Numeric::Min(const FieldValue& field) {
  if (std::holds_alternative<std::monostate>(v_)) {
    if (std::optional<int64_t> that = TryParseAs<int64_t>(field)) {
      v_ = *that;
    } else {
      v_ = ParseAs<double>(field);
    }
    return;
  }
  if (std::holds_alternative<int64_t>(v_)) {
    if (std::optional<int64_t> that = TryParseAs<int64_t>(field)) {
      std::get<int64_t>(v_) = std::min(std::get<int64_t>(v_), *that);
      return;
    }
    v_ = static_cast<double>(std::get<int64_t>(v_));
  }
  std::get<double>(v_) = std::min(std::get<double>(v_), ParseAs<double>(field));
}

void Numeric::Max(const FieldValue& field) {
  if (std::holds_alternative<std::monostate>(v_)) {
    if (std::optional<int64_t> that = TryParseAs<int64_t>(field)) {
      v_ = *that;
    } else {
      v_ = ParseAs<double>(field);
    }
    return;
  }
  if (std::holds_alternative<int64_t>(v_)) {
    if (std::optional<int64_t> that = TryParseAs<int64_t>(field)) {
      std::get<int64_t>(v_) = std::max(std::get<int64_t>(v_), *that);
      return;
    }
    v_ = static_cast<double>(std::get<int64_t>(v_));
  }
  std::get<double>(v_) = std::max(std::get<double>(v_), ParseAs<double>(field));
}

void Numeric::Print(std::string* out) const {
  struct {
    void operator()(std::monostate) { Fail("Uninitialized value?"); }
    void operator()(int64_t v) { absl::StrAppend(out, v); }
    void operator()(double v) { absl::StrAppendFormat(out, "%.8g", v); }
    std::string* out;
  } p{out};
  std::visit(p, v_);
}
