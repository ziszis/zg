#include "pipeline.h"

#include "aggregators.h"
#include "composite-key.h"
#include "filter-table.h"
#include "multi-aggregation.h"
#include "no-keys.h"
#include "output.h"
#include "simple-table.h"
#include "single-key.h"

using namespace spec;

namespace {

int NumColumns(const AggregatedTable::Component& cmp) {
  struct {
    int operator()(const Key&) { return 1; }
    int operator()(const Sum&) { return 1; }
    int operator()(const Min& m) { return std::max<int>(1, m.output.size()); }
    int operator()(const Max& m) { return std::max<int>(1, m.output.size()); }
    int operator()(const Count&) { return 1; }
    int operator()(const CountDistinct&) { return 1; }
  } v;
  return std::visit(v, cmp);
}

std::unique_ptr<Table> BuildNoAggregationTable(
    std::vector<Table::Key> keys, std::unique_ptr<OutputTable> output) {
  if (keys.empty()) LogicError("aggregated table with no columns");
  if (keys.size() == 1) {
    return std::make_unique<SingleKeyNoAggregationTable>(keys[0],
                                                         std::move(output));
  } else {
    return std::make_unique<CompositeKeyNoAggregationTable>(std::move(keys),
                                                            std::move(output));
  }
}

template <class Aggregator>
std::unique_ptr<Table> BuildSingleAggregatorTable(
    std::vector<Table::Key> keys, Aggregator aggregator,
    std::unique_ptr<OutputTable> output) {
  if (keys.empty()) {
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

auto AggregatorFromSpec(int column, const Key& s) -> CountAggregator {
  LogicError("key as aggregator");
}

auto AggregatorFromSpec(int column, const Sum& s) {
  return SumAggregator<Numeric>(s.expr.field, column);
}

auto AggregatorFromSpec(int column, const Min& m) {
  if (!m.output.empty()) Unimplemented("minarg");
  return MinAggregator<Numeric>(m.what.field, column);
}

auto AggregatorFromSpec(int column, const Max& m) {
  if (!m.output.empty()) Unimplemented("maxarg");
  return MaxAggregator<Numeric>(m.what.field, column);
}

auto AggregatorFromSpec(int column, const Count&) {
  return CountAggregator(column);
}

auto AggregatorFromSpec(int column, const CountDistinct&) -> CountAggregator {
  Unimplemented("count(distinct)");
}

std::unique_ptr<Table> AggregateFromSpec(
    const std::vector<AggregatedTable::Component>& components,
    std::unique_ptr<Table> pipe_to) {
  if (components.empty()) LogicError("aggregated table with no columns");

  std::vector<Table::Key> keys;
  int num_columns = 0;
  int num_aggs = 0;
  for (const auto& cmp : components) {
    if (const spec::Key* key = std::get_if<spec::Key>(&cmp)) {
      keys.push_back(Table::Key(key->expr.field, num_columns));
    } else {
      ++num_aggs;
    }
    num_columns += NumColumns(cmp);
  }

  std::unique_ptr<OutputTable> output =
      pipe_to ? MakePipeTable(num_columns, std::move(pipe_to))
              : MakeStdoutTable(num_columns);

  if (num_aggs == 0) {
    return BuildNoAggregationTable(std::move(keys), std::move(output));
  } else if (num_aggs == 1) {
    int agg_column = 0;
    for (const auto& cmp : components) {
      if (!std::holds_alternative<spec::Key>(cmp)) {
        return std::visit(
            [&](auto&& agg_spec) {
              auto agg = AggregatorFromSpec(agg_column, agg_spec);
              return BuildSingleAggregatorTable(std::move(keys), std::move(agg),
                                                std::move(output));
            },
            cmp);
      }
      agg_column += NumColumns(cmp);
    }
    LogicError("cannot find aggregator");
  } else {
    std::vector<std::unique_ptr<AggregatorInterface>> aggregators;
    int agg_column = 0;
    for (const auto& cmp : components) {
      if (!std::holds_alternative<spec::Key>(cmp)) {
        std::visit(
            [&](auto&& agg_spec) {
              auto agg = AggregatorFromSpec(agg_column, std::move(agg_spec));
              aggregators.push_back(TypeErasedAggregator(std::move(agg)));
            },
            cmp);
      }
      agg_column += NumColumns(cmp);
    }
    return MakeMultiAggregatorTable(std::move(keys), std::move(aggregators),
                                    std::move(output));
  }
}

std::unique_ptr<Table> TableFromSpec(const spec::AggregatedTable& spec,
                                     std::unique_ptr<Table> pipe_to) {
  return WrapFilter(spec.filters,
                    AggregateFromSpec(spec.components, std::move(pipe_to)));
}

std::unique_ptr<Table> TableFromSpec(const spec::SimpleTable& spec,
                                     std::unique_ptr<Table> pipe_to) {
  if (spec.columns.empty()) {
    // We must be the last table, otherwise Optimize() should have fused us.
    if (pipe_to) LogicError("implicit output inside the pipeline");
    return WrapFilter(spec.filters, MakePassthroughTable());
  }
  std::unique_ptr<OutputTable> output =
      pipe_to ? MakePipeTable(spec.columns.size(), std::move(pipe_to))
              : MakeStdoutTable(spec.columns.size());
  return WrapFilter(spec.filters,
                    MakeSimpleTable(spec.columns, std::move(output)));
}

void MoveFilters(SimpleTable* from, Stage* to) {
  struct {
    auto operator()(AggregatedTable& t) { return &t.filters; }
    auto operator()(SimpleTable& t) { return &t.filters; }
  } v;
  std::move(from->filters.begin(), from->filters.end(),
            std::back_inserter(*std::visit(v, *to)));
  from->filters.clear();
}

void OptimizeSpec(Pipeline* spec) {
  // If a SimpleTable has no output columns, fuse its filters into the next
  // stage.
  for (int i = 0; i < spec->size(); ++i) {
    if (SimpleTable* table = std::get_if<SimpleTable>(&(*spec)[i])) {
      if (table->columns.empty() && i + 1 < spec->size()) {
        MoveFilters(table, &(*spec)[i + 1]);
      }
    }
  }

  // Remove no-op tables.
  spec->erase(std::remove_if(
                  spec->begin(), spec->end(),
                  [](const Stage& stage) {
                    if (const auto* table = std::get_if<SimpleTable>(&stage)) {
                      return table->columns.empty() && table->filters.empty();
                    }
                    return false;
                  }),
              spec->end());
  if (spec->empty()) {
    spec->push_back(SimpleTable{});
  }
}

}  // namespace

std::unique_ptr<Table> BuildPipeline(spec::Pipeline spec) {
  OptimizeSpec(&spec);
  std::unique_ptr<Table> result;
  for (int i = spec.size(); i-- > 0;) {
    std::visit(
        [&](auto&& stage) { result = TableFromSpec(stage, std::move(result)); },
        spec[i]);
  }
  return result;
}
