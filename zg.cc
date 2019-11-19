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

template <class Aggregator>
class SingleAggregatorTable {
 public:
  SingleAggregatorTable(int key_field, int value_field,
                        Aggregator agg = Aggregator())
      : key_field_(key_field), value_field_(value_field), agg_(agg) {}

  void AddRow(const std::vector<FieldValue>& fields) {
    auto [it, inserted] = rows_.try_emplace(fields[key_field_].AsString());
    if (inserted) {
      agg_.Init(&it->second);
    }
    agg_.Push(fields[value_field_], &it->second);
  }

  void Render() {
    std::string buffer;
    for (const auto& [key, value] : rows_) {
      buffer.append(key);
      buffer.push_back('\t');
      agg_.Print(value, &buffer);
      buffer.push_back('\n');
      if (buffer.size() > 1 << 15) {
        if (std::fwrite(buffer.data(), 1, buffer.size(), stdout) !=
            buffer.size()) {
          Fail("Write failed");
        }
        buffer.clear();
      }
    }
    if (std::fwrite(buffer.data(), 1, buffer.size(), stdout) != buffer.size()) {
      Fail("Write failed");
    }
  }

 private:
  absl::flat_hash_map<std::string, typename Aggregator::State> rows_;
  int key_field_;
  int value_field_;
  Aggregator agg_;
};

void SplitLine(std::string_view line, std::vector<FieldValue>* fields) {
  // Using for-loop instead of the more canonical `fields = absl::StrSplit(...)`
  // to avoid vector reallocations.
  fields->clear();
  for (auto field :
       absl::StrSplit(line, absl::ByAnyChar(" \t"), absl::SkipWhitespace())) {
    fields->emplace_back(field);
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
        lineFn(std::string_view(begin, p - begin));
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
          lineFn(std::string_view(begin, bufferLeft));
        }
        return;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  std::vector<FieldValue> fields;
  SingleAggregatorTable<CountAggregator> table(1, 1);
  ForEachInputLine([&](std::string_view line) {
    SplitLine(line, &fields);
    table.AddRow(fields);
  });
  table.Render();
  return 0;
}
