#include "spec.h"

#include <charconv>

#include "aggregators.h"
#include "base.h"
#include "composite-key.h"
#include "multi-aggregation.h"
#include "no-keys.h"
#include "single-key.h"

namespace {

template <class Fn>
class Parser {
 public:
  Parser(const std::vector<std::string>& spec, std::vector<Table::Key>* keys,
         const Fn& fn)
      : token_(spec.begin()), tokens_end_(spec.end()), keys_(keys), fn_(fn) {}

  void Spec() {
    while (token_ != tokens_end_ || char_ != nullptr) {
      if (TryConsume('k', "key")) {
        keys_->emplace_back(ConsumeInt() - 1, next_output_column_++);
      } else if (TryConsume('c', "count")) {
        fn_(CountAggregator(next_output_column_++));
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
      if (token_ == tokens_end_ || *token_ != token) return false;
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
        Fail("Expected integer, got ",
             Quoted(std::string_view(char_, chars_end_ - char_)));
      }
    } else {
      if (token_ == tokens_end_) Fail("Expected integer");
      if (!absl::SimpleAtoi(*token_, &result)) {
        Fail("Failed to parse integer: ", Quoted(*token_));
      }
      ++token_;
    }
    return result;
  }

  template <template <class V> class A>
  void WithTypeAndField() {
    if (TryConsume('i', "int")) {
      fn_(A<NativeNum<int64_t>>(ConsumeInt() - 1, next_output_column_++));
    } else if (TryConsume('d', "double")) {
      fn_(A<NativeNum<double>>(ConsumeInt() - 1, next_output_column_++));
    } else {
      fn_(A<Numeric>(ConsumeInt() - 1, next_output_column_++));
    }
  }

  std::vector<std::string>::const_iterator token_;
  std::vector<std::string>::const_iterator tokens_end_;
  const char* char_ = nullptr;
  const char* chars_end_ = nullptr;

  int next_output_column_ = 0;
  std::vector<Table::Key>* keys_;
  const Fn& fn_;
};

template <class Fn>
void ParseSpec(const std::vector<std::string>& spec,
               std::vector<Table::Key>* keys, const Fn& fn) {
  std::vector<Table::Key> dummy;
  Parser<Fn>(spec, keys ? keys : &dummy, fn).Spec();
}

}  // namespace

std::unique_ptr<Table> ParseSpec(const std::vector<std::string>& spec) {
  std::vector<Table::Key> keys;
  int num_agg = 0;
  ParseSpec(spec, &keys, [&](auto) { ++num_agg; });

  if (num_agg == 0) {
    if (keys.size() == 0) {
      Fail("Nothing to do");
    } else if (keys.size() == 1) {
      return std::make_unique<SingleKeyNoAggregationTable>(keys[0]);
    } else {
      return std::make_unique<CompositeKeyNoAggregationTable>(std::move(keys));
    }
  } else if (num_agg == 1) {
    std::unique_ptr<Table> result;
    ParseSpec(spec, nullptr, [&](auto agg) {
      if (keys.size() == 0) {
        result = std::make_unique<NoKeyTable<decltype(agg)>>(std::move(agg));
      } else if (keys.size() == 1) {
        result = std::make_unique<SingleKeyTable<decltype(agg)>>(
            keys[0], std::move(agg));
      } else {
        result = std::make_unique<CompositeKeyTable<decltype(agg)>>(
            std::move(keys), std::move(agg));
      }
    });
    return result;
  } else {
    std::vector<std::unique_ptr<AggregatorInterface>> aggs;
    ParseSpec(spec, nullptr, [&](auto agg) {
      aggs.push_back(TypeErasedAggregator(std::move(agg)));
    });
    return MakeMultiAggregatorTable(std::move(keys), std::move(aggs));
  }
}
