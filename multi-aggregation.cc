#include "multi-aggregation.h"

namespace {

struct AggregatorField {
  std::unique_ptr<AggregatorInterface> aggregator;
  size_t state_offset;
};

// A simple wrapper around char[size]. Needed because std::optional doesn't
// allow naked arrays.
template <int size>
class Chars {
 public:
  char& operator[](size_t offset) { return buf[offset]; }
  const char& operator[](size_t offset) const { return buf[offset]; }

 private:
  char buf[size];
};

template <int state_size, class State>
class MultiAggregatorTable : public Table {
 public:
  using Value = Chars<state_size>;

  MultiAggregatorTable(State state, std::vector<AggregatorField> fields)
      : state_(std::move(state)), fields_(std::move(fields)) {}

  void PushRow(const InputRow& row) override {
    state_.Push(
        row,
        [&] {
          Value value;
          for (const auto& f : fields_) {
            f.aggregator->Init(row, &value[f.state_offset]);
          }
          return value;
        },
        [&](Value& value) {
          for (const auto& f : fields_) {
            f.aggregator->Update(row, &value[f.state_offset]);
          }
        });
  }

  void Render() override {
    OutputBuffer buffer;
    state_.Render(&buffer, [&](const Value& value) {
      for (const auto& f : fields_) {
        f.aggregator->Print(&value[f.state_offset], &buffer);
      }
    });
  }

 private:
  State state_;
  std::vector<AggregatorField> fields_;
};

std::pair<size_t, std::vector<AggregatorField>> LayoutAggregatorState(
    std::vector<std::unique_ptr<AggregatorInterface>> aggs) {
  std::vector<AggregatorField> fields;
  for (std::unique_ptr<AggregatorInterface>& agg : aggs) {
    fields.emplace_back();
    fields.back().state_offset = agg->StateSize();
    fields.back().aggregator = std::move(agg);
  }
  std::sort(fields.begin(), fields.end(), [](const auto& a, const auto& b) {
    return a.state_offset > b.state_offset;
  });
  size_t size = 0;
  for (auto& f : fields) {
    size_t this_size = f.state_offset;
    f.state_offset = size;
    size += this_size;
  }
  return {size, std::move(fields)};
}

template <int state_size, template <class Value> class State, class... Args>
std::unique_ptr<Table> MakeMultiAggregatorTable(
    std::vector<AggregatorField> fields, Args&&... state_args) {
  return std::make_unique<
      MultiAggregatorTable<state_size, State<Chars<state_size>>>>(
      State<Chars<state_size>>(std::forward<Args>(state_args)...),
      std::move(fields));
}

template <int state_size>
std::unique_ptr<Table> MakeMultiAggregatorTableWithSize(
    const std::vector<AggregationState::Key>& keys,
    std::vector<AggregatorField> fields) {
  if (keys.size() == 0) {
    return MakeMultiAggregatorTable<state_size, AggregationState::NoKeys>(
        std::move(fields));
  } else if (keys.size() == 1) {
    return MakeMultiAggregatorTable<state_size, AggregationState::SingleKey>(
        std::move(fields), keys[0]);
  } else {
    return MakeMultiAggregatorTable<state_size, AggregationState::CompositeKey>(
        std::move(fields), keys);
  }
}

}  // namespace

std::unique_ptr<Table> MakeMultiAggregatorTable(
    const std::vector<AggregationState::Key>& keys,
    std::vector<std::unique_ptr<AggregatorInterface>> aggregators) {
  auto [total_size, fields] = LayoutAggregatorState(std::move(aggregators));

  if (total_size <= 8) {
    return MakeMultiAggregatorTableWithSize<8>(keys, std::move(fields));
  } else if (total_size <= 16) {
    return MakeMultiAggregatorTableWithSize<16>(keys, std::move(fields));
  } else if (total_size <= 24) {
    return MakeMultiAggregatorTableWithSize<24>(keys, std::move(fields));
  } else if (total_size <= 32) {
    return MakeMultiAggregatorTableWithSize<32>(keys, std::move(fields));
  } else if (total_size <= 48) {
    return MakeMultiAggregatorTableWithSize<48>(keys, std::move(fields));
  } else if (total_size <= 64) {
    return MakeMultiAggregatorTableWithSize<64>(keys, std::move(fields));
  } else {
    Fail("Too much state");  // Add more branches.
  }
}
