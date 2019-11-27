#include "spec.h"

#include <charconv>

#include "aggregators.h"
#include "base.h"
#include "multi-aggregation.h"
#include "single-aggregation.h"

namespace {

template <class Fn>
class ShorthandParser {
 public:
  ShorthandParser(std::string_view token, std::vector<int>* key_fields,
                  const Fn& fn)
      : it_(token.data()),
        end_(token.end()),
        key_fields_(key_fields),
        fn_(fn) {}

  void Spec() {
    while (it_ != end_) {
      if (TryConsume('k')) {
        key_fields_->push_back(ConsumeInt() - 1);
      } else if (TryConsume('c')) {
        fn_(CountAggregator());
      } else if (TryConsume('s')) {
        fn_(SumAggregator<int64_t>(ConsumeInt() - 1));
      } else if (TryConsume('m')) {
        fn_(MinAggregator<int64_t>(ConsumeInt() - 1));
      } else if (TryConsume('M')) {
        fn_(MaxAggregator<int64_t>(ConsumeInt() - 1));
      } else {
        Fail("Unrecognized operation");
      }
    }
  }

 private:
  bool TryConsume(char ch) {
    if (*it_ != ch) return false;
    ++it_;
    return true;
  }

  int ConsumeInt() {
    int result;
    auto [ptr, ec] = std::from_chars(it_, end_, result);
    if (ec == std::errc()) {
      it_ = ptr;
      return result;
    } else {
      Fail("Expected integer");
      return 0;
    }
  }

  const char* it_;
  const char* end_;
  std::vector<int>* key_fields_;
  const Fn& fn_;
};

template <class Fn>
class Parser {
 public:
  Parser(const std::vector<std::string>& spec, std::vector<int>* key_fields,
         const Fn& fn)
      : it_(spec.begin()), end_(spec.end()), key_fields_(key_fields), fn_(fn) {}

  void Spec() {
    while (it_ != end_) {
      if (TryConsume("key")) {
        key_fields_->push_back(ConsumeInt() - 1);
      } else if (TryConsume("count")) {
        fn_(CountAggregator());
      } else if (TryConsume("sum")) {
        fn_(SumAggregator<int64_t>(ConsumeInt() - 1));
      } else if (TryConsume("min")) {
        fn_(MinAggregator<int64_t>(ConsumeInt() - 1));
      } else if (TryConsume("max")) {
        fn_(MaxAggregator<int64_t>(ConsumeInt() - 1));
      } else {
        ShorthandParser<Fn>(*it_++, key_fields_, fn_).Spec();
      }
    }
  }

 private:
  bool TryConsume(const char* token) {
    if (*it_ != token) return false;
    ++it_;
    return true;
  }

  int ConsumeInt() {
    if (it_ == end_) {
      Fail("Expected integer");
    }
    int result;
    if (!absl::SimpleAtoi(*it_, &result)) {
      Fail("Failed to parse integer");
    }
    ++it_;
    return result;
  }

  std::vector<std::string>::const_iterator it_;
  std::vector<std::string>::const_iterator end_;
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
