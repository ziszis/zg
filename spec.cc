#include "spec.h"

#include <charconv>

#include "aggregators.h"
#include "base.h"
#include "multi-aggregation.h"
#include "single-aggregation.h"

namespace {

template <class Fn>
class Parser {
 public:
  Parser(const std::vector<std::string>& spec, std::vector<int>* key_fields,
         const Fn& fn)
      : token_(spec.begin()),
        tokens_end_(spec.end()),
        key_fields_(key_fields),
        fn_(fn) {}

  void Spec() {
    while (token_ != tokens_end_ || char_ != nullptr) {
      if (TryConsume('k', "key")) {
        key_fields_->push_back(ConsumeInt() - 1);
      } else if (TryConsume('c', "count")) {
        fn_(CountAggregator());
      } else if (TryConsume('s', "sum")) {
        WithTypeAndField<SumAggregator>();
      } else if (TryConsume('m', "min")) {
        WithTypeAndField<MinAggregator>();
      } else if (TryConsume('M', "max")) {
        WithTypeAndField<MaxAggregator>();
      } else if (char_ == nullptr) {
        char_ = token_->data();
        chars_end_ = char_ + token_->size();
        ++token_;
      } else {
        Fail("Unparseable spec");
      }
    }
  }

 private:
  bool TryConsume(char ch, const char* token) {
    if (char_) {
      if (*char_ != ch) return false;
      if (++char_ == chars_end_) char_ = nullptr;
      return true;
    } else {
      if (*token_ != token) return false;
      ++token_;
      return true;
    }
  }

  int ConsumeInt() {
    int result = 0;
    if (char_) {
      auto [ptr, ec] = std::from_chars(char_, chars_end_, result);
      if (ec == std::errc()) {
        char_ = ptr;
        if (char_ == chars_end_) char_ = nullptr;
      } else {
        Fail("Expected integer");
      }
    } else {
      if (token_ == tokens_end_) Fail("Expected integer");
      if (!absl::SimpleAtoi(*token_, &result)) Fail("Failed to parse integer");
      ++token_;
    }
    return result;
  }

  template <template <class V> class A>
  void WithTypeAndField() {
    if (TryConsume('i', "int")) {
      fn_(A<NativeNum<int64_t>>(ConsumeInt() - 1));
    } else if (TryConsume('d', "double")) {
      fn_(A<NativeNum<double>>(ConsumeInt() - 1));
    } else {
      fn_(A<Numeric>(ConsumeInt() - 1));
    }
  }

  std::vector<std::string>::const_iterator token_;
  std::vector<std::string>::const_iterator tokens_end_;
  const char* char_ = nullptr;
  const char* chars_end_ = nullptr;

  std::vector<int>* key_fields_;
  const Fn& fn_;
};

template <class Fn>
void ParseSpec(const std::vector<std::string>& spec,
               std::vector<int>* key_fields, const Fn& fn) {
  std::vector<int> dummy;
  Parser<Fn>(spec, key_fields ? key_fields : &dummy, fn).Spec();
}

}  // namespace

std::unique_ptr<Table> ParseSpec(const std::vector<std::string>& spec) {
  std::vector<int> key_fields;
  int num_agg = 0;
  ParseSpec(spec, &key_fields, [&](auto) { ++num_agg; });

  std::unique_ptr<Table> result;
  if (num_agg == 0) {
    Fail("Not implemented");
  } else if (num_agg == 1) {
    ParseSpec(spec, nullptr, [&](auto agg) {
      result = MakeSingleAggregatorTable(key_fields, std::move(agg));
    });
  } else {
    std::vector<std::unique_ptr<AggregatorInterface>> aggs;
    ParseSpec(spec, nullptr, [&](auto agg) {
      aggs.push_back(TypeErasedAggregator(std::move(agg)));
    });
    result = MakeMultiAggregatorTable(key_fields, std::move(aggs));
  }
  return result;
}
