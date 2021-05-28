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

  Value Eval(const InputRow& row) const { return Value::Make(row[field]); }
};

template <class Value>
class ExprColumn {
 public:
  ExprColumn(int column, const spec::Expr& spec)
      : expr_(Expr<Value>::FromSpec(spec)), column_(column) {}

  Value Eval(const InputRow& row) const { return expr_.Eval(row); }

  void Print(const Value& value, OutputTable& out) const {
    buf_.clear();
    value.Print(&buf_);
    out.Set(column_, buf_);
  }

  void Reset() const { decltype(buf_)().swap(buf_); }

  static std::vector<ExprColumn<Value>> FromSpecs(
      int first_column, const std::vector<spec::Expr>& spec) {
    std::vector<ExprColumn<Value>> result;
    for (const auto& s : spec) {
      result.push_back(ExprColumn(first_column++, s));
    }
    return result;
  }

 private:
  Expr<Value> expr_;
  int column_;
  mutable std::string buf_;
};

#endif  // GITHUB_ZISZIS_ZG_EXPR_INCLUDED
