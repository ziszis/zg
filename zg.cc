#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"

void Fail(std::string_view reason) {
  std::cerr << reason << std::endl;
  std::abort();
}

class FieldValue {
 public:
  explicit FieldValue(std::string_view value) : value_(value) {}

  std::string_view AsString() const { return value_; }

  int64_t AsInt64() const {
    if (!int_) {
      int_.emplace();
      if (!absl::SimpleAtoi(value_, &*int_)) {
        Fail("Failed to parse int");
      }
    }
    return *int_;
  }

 private:
  std::string_view value_;
  mutable std::optional<int64_t> int_;
};

class CountAggregator {
 public:
  using State = int64_t;
  void Init(State* state) const { *state = 0; }
  void Push(const FieldValue&, State* state) const { ++*state; }
  void Print(State state, std::string* out) const {
    absl::StrAppend(out, state);
  }
};

class OutputBuffer {
 public:
  std::string* raw() { return &buf_; }
  void MaybeFlush() {
    if (buf_.size() > 1 << 15) Flush();
  }
  ~OutputBuffer() { Flush(); }

 private:
  void Flush() {
    if (std::fwrite(buf_.data(), 1, buf_.size(), stdout) != buf_.size()) {
      Fail("Write failed");
    }
    buf_.clear();
  }

  std::string buf_;
};

class Table {
 public:
  virtual ~Table() {}
  virtual void PushRow(const std::vector<FieldValue>& fields) = 0;
  virtual void Render() = 0;
};

template <class Aggregator>
class NoGroupingTable : public Table {
 public:
  NoGroupingTable(int value_field, Aggregator agg = Aggregator())
      : value_field_(value_field), agg_(agg) {
    agg_.Init(&state_);
  }

  void PushRow(const std::vector<FieldValue>& fields) override {
    agg_.Push(fields[value_field_], &state_);
  }

  void Render() override {
    OutputBuffer buffer;
    agg_.Print(state_, buffer.raw());
    buffer.raw()->push_back('\n');
  }

 private:
  typename Aggregator::State state_;
  int value_field_;
  Aggregator agg_;
};

template <class Aggregator>
class SingleAggregatorTable : public Table {
 public:
  SingleAggregatorTable(int key_field, int value_field,
                        Aggregator agg = Aggregator())
      : key_field_(key_field), value_field_(value_field), agg_(agg) {}

  void PushRow(const std::vector<FieldValue>& fields) override {
    auto [it, inserted] = rows_.try_emplace(fields[key_field_].AsString());
    if (inserted) {
      agg_.Init(&it->second);
    }
    agg_.Push(fields[value_field_], &it->second);
  }

  void Render() override {
    OutputBuffer buffer;
    for (const auto& [key, value] : rows_) {
      buffer.raw()->append(key);
      buffer.raw()->push_back('\t');
      agg_.Print(value, buffer.raw());
      buffer.raw()->push_back('\n');
      buffer.MaybeFlush();
    }
  }

 private:
  absl::flat_hash_map<std::string, typename Aggregator::State> rows_;
  int key_field_;
  int value_field_;
  Aggregator agg_;
};

void SplitLine(const char* line, const char* end,
               std::vector<FieldValue>* fields) {
  fields->clear();
  const char* begin = line;
  while (true) {
    while (true) {
      if (begin == end) return;
      if (*begin != ' ' && *begin != '\t') break;
      ++begin;
    }
    const char* p = begin;
    while (true) {
      if (p == end) {
        fields->emplace_back(std::string_view(begin, p - begin));
        return;
      }
      if (*p == ' ' || *p == '\t') {
        fields->emplace_back(std::string_view(begin, p - begin));
        begin = p + 1;
        break;
      }
      ++p;
    }
  }
}

template <class LineFn>
void ForEachInputLine(LineFn lineFn) {
  std::string buffer(16384, '\0');
  char* begin = buffer.data();
  size_t bufferLeft = 0;

  while (true) {
    size_t wantToRead = buffer.size() - bufferLeft;
    size_t actuallyRead = std::fread(begin + bufferLeft, 1, wantToRead, stdin);
    bufferLeft += actuallyRead;
    while (true) {
      char* p = static_cast<char*>(memchr(begin, '\n', bufferLeft));
      if (p != nullptr) {
        lineFn(begin, p);
        ++p;
        bufferLeft -= p - begin;
        begin = p;
      } else if (wantToRead == actuallyRead) {
        memmove(buffer.data(), begin, bufferLeft);
        if (bufferLeft > buffer.size() / 4) {
          buffer.resize(buffer.size() * 4, '\0');
        }
        begin = buffer.data();
        // TODO: Shrink buffer if too little of it is used?
        break;
      } else {
        if (bufferLeft != 0) {
          lineFn(begin, begin + bufferLeft);
        }
        return;
      }
    }
  }
}

std::unique_ptr<Table> ParseSpec(absl::string_view spec) {
  if (spec == "k2c") {
    return std::make_unique<SingleAggregatorTable<CountAggregator>>(1, 1);
  } else if (spec == "k1c") {
    return std::make_unique<SingleAggregatorTable<CountAggregator>>(0, 0);
  } else if (spec == "c") {
    return std::make_unique<NoGroupingTable<CountAggregator>>(0);
  } else {
    Fail("Unsupported spec");
    return nullptr;
  }
}

int main(int argc, char* argv[]) {
  std::vector<FieldValue> fields;
  if (argc < 2) Fail("Need spec");
  std::unique_ptr<Table> table = ParseSpec(argv[1]);
  ForEachInputLine([&](const char* begin, const char* end) {
    SplitLine(begin, end, &fields);
    table->PushRow(fields);
  });
  table->Render();
  return 0;
}
