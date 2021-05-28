#ifndef GITHUB_ZISZIS_ZG_EXPR_INCLUDED
#define GITHUB_ZISZIS_ZG_EXPR_INCLUDED

#include <vector>

#include "output.h"
#include "spec.h"
#include "types.h"

template<class Value>
struct Expr {
  int field;

  static Expr FromSpec(const spec::Expr& spec) { return {.field = spec.field}; }

  Value Eval(const InputRow& row) const;
};

template <class Value>
class ExprColumn {
 public:
  ExprColumn(int column, const spec::Expr& spec)
      : expr_(Expr<Value>::FromSpec(spec)), column_(column) {}

  Value Eval(const InputRow& row) const { return expr_.Eval(row); }
  void Print(const Value& value, OutputTable& out) const;

  void Reset() const { decltype(buf_)().swap(buf_); }

  static std::vector<ExprColumn<Value>> FromSpecs(
      int first_column, const std::vector<spec::Expr>& spec);

 private:
  Expr<Value> expr_;
  int column_;
  mutable std::string buf_;
};

// ========================================================================
// Implementation details
// ========================================================================

template <>
inline std::string_view Expr<std::string_view>::Eval(
    const InputRow& row) const {
  return row[field];
}

template <class T>
inline T Expr<T>::Eval(const InputRow& row) const {
  return T::Make(row[field]);
}

template <class T>
inline void ExprColumn<T>::Print(const T& value, OutputTable& out) const {
  buf_.clear();
  value.Print(&buf_);
  out.Set(column_, buf_);
}

template <>
inline void ExprColumn<std::string_view>::Print(const std::string_view& value,
                                                OutputTable& out) const {
  out.Set(column_, value);
}

template <class Value>
std::vector<ExprColumn<Value>> ExprColumn<Value>::FromSpecs(
    int first_column, const std::vector<spec::Expr>& spec) {
  std::vector<ExprColumn<Value>> result;
  for (const auto& s : spec) {
    result.push_back(ExprColumn(first_column++, s));
  }
  return result;
}

#endif  // GITHUB_ZISZIS_ZG_EXPR_INCLUDED
