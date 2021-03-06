#include "multi-aggregation.h"

#include "composite-key.h"
#include "no-keys.h"
#include "single-key.h"

namespace {

struct AggregatorField {
  std::unique_ptr<AggregatorInterface> aggregator;
  size_t state_offset;
};

// A simple wrapper around char[size]. Needed because std::optional doesn't
// allow naked arrays.
//
// alignas(8) can be relaxed if needed, but 1) it's not needed currently
// useful, and 2) along with static_assert(alignof(State) <= 8)) guarantees
// correct alignment of all states without complicating LayoutAggregatorState()
template <int size>
class alignas(8) Chars {
 public:
  char& operator[](size_t offset) { return buf[offset]; }
  const char& operator[](size_t offset) const { return buf[offset]; }

 private:
  char buf[size];
};

template <int size>
class MultiAggregator {
 public:
  using State = Chars<size>;

  explicit MultiAggregator(std::vector<AggregatorField> fields)
      : fields_(std::move(fields)) {}

  State Init(const InputRow& row) const {
    State state;
    for (const auto& f : fields_) {
      f.aggregator->Init(row, &state[f.state_offset]);
    }
    return state;
  }

  void Update(const InputRow& row, State& state) const {
    for (const auto& f : fields_) {
      f.aggregator->Update(row, &state[f.state_offset]);
    }
  }

  void Print(const State& state, OutputTable& out) const {
    for (const auto& f : fields_) {
      f.aggregator->Print(&state[f.state_offset], out);
    }
  }

  void Reset() const {
    for (const auto& f : fields_) {
      f.aggregator->Reset();
    }
  }

 private:
  std::vector<AggregatorField> fields_;
};

std::pair<size_t, std::vector<AggregatorField>> LayoutAggregatorState(
    std::vector<std::unique_ptr<AggregatorInterface>> aggs) {
  std::vector<AggregatorField> fields;
  for (std::unique_ptr<AggregatorInterface>& agg : aggs) {
    fields.emplace_back();
    fields.back().aggregator = std::move(agg);
  }
  std::sort(fields.begin(), fields.end(), [](const auto& a, const auto& b) {
    return a.aggregator->StateSize() > b.aggregator->StateSize();
  });
  size_t offset = 0;
  for (auto& f : fields) {
    size_t align = f.aggregator->StateAlign();
    offset = (offset + align - 1) / align * align;
    f.state_offset = offset;
    offset += f.aggregator->StateSize();
  }
  return {offset, std::move(fields)};
}

template <class Aggregator>
std::unique_ptr<Table> MakeMultiAggregatorTable(
    Aggregator aggregator, std::vector<Table::Key> keys,
    std::unique_ptr<OutputTable> output) {
  if (keys.size() == 0) {
    return std::make_unique<NoKeyTable<Aggregator>>(std::move(aggregator),
                                                    std::move(output));
  } else if (keys.size() == 1) {
    return std::make_unique<SingleKeyTable<Aggregator>>(
        keys[0], std::move(aggregator), std::move(output));
  } else {
    return std::make_unique<CompositeKeyTable<Aggregator>>(
        std::move(keys), std::move(aggregator), std::move(output));
  }
}

}  // namespace

std::unique_ptr<Table> MakeMultiAggregatorTable(
    std::vector<Table::Key> keys,
    std::vector<std::unique_ptr<AggregatorInterface>> aggregators,
    std::unique_ptr<OutputTable> output) {
  auto [total_size, fields] = LayoutAggregatorState(std::move(aggregators));

  if (total_size <= 8) {
    return MakeMultiAggregatorTable(MultiAggregator<8>(std::move(fields)),
                                    std::move(keys), std::move(output));
  } else if (total_size <= 16) {
    return MakeMultiAggregatorTable(MultiAggregator<16>(std::move(fields)),
                                    std::move(keys), std::move(output));
  } else if (total_size <= 24) {
    return MakeMultiAggregatorTable(MultiAggregator<24>(std::move(fields)),
                                    std::move(keys), std::move(output));
  } else if (total_size <= 32) {
    return MakeMultiAggregatorTable(MultiAggregator<32>(std::move(fields)),
                                    std::move(keys), std::move(output));
  } else if (total_size <= 48) {
    return MakeMultiAggregatorTable(MultiAggregator<48>(std::move(fields)),
                                    std::move(keys), std::move(output));
  } else if (total_size <= 64) {
    return MakeMultiAggregatorTable(MultiAggregator<64>(std::move(fields)),
                                    std::move(keys), std::move(output));
  } else {
    Fail("Too much state");  // Add more branches.
  }
}
