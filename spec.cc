#include "spec.h"

#include "aggregators.h"
#include "multi-aggregation.h"
#include "single-aggregation.h"

namespace {

template <class Fn>
void ParseSpec(const std::vector<std::string>& spec,
               std::vector<int>* key_fields, Fn fn) {
  key_fields->clear();
  if (spec[0] == "k2c") {
    key_fields->push_back(1);
    fn(CountAggregator());
  } else if (spec[0] == "k1c") {
    key_fields->push_back(0);
    fn(CountAggregator());
  } else if (spec[0] == "k2s1") {
    key_fields->push_back(1);
    fn(SumAggregator<int64_t>(0));
  } else if (spec[0] == "k2sf1") {
    key_fields->push_back(1);
    fn(SumAggregator<double>(0));
  } else if (spec[0] == "c") {
    fn(CountAggregator());
  } else {
    Fail("Unsupported spec");
  }
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
    ParseSpec(spec, &key_fields, [&](auto agg) {
      result = MakeSingleAggregatorTable(key_fields, std::move(agg));
    });
  } else {
    std::vector<std::unique_ptr<AggregatorInterface>> aggs;
    ParseSpec(spec, &key_fields, [&](auto agg) {
      aggs.push_back(TypeErasedAggregator(std::move(agg)));
    });
    result = MakeMultiAggregatorTable(key_fields, std::move(aggs));
  }
  return result;
}
