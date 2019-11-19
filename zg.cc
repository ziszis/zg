#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/container/flat_hash_map.h"

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

class Aggregator {
 public:
  virtual ~Aggregator() {}
  virtual size_t StateSize() const = 0;
  virtual void Init(char* state) = 0;
  virtual void Push(const FieldValue&, char* state) = 0;
  virtual void Print(const char* state, std::string* out) const = 0;
};

class CountAggregator : public Aggregator {
  size_t StateSize() const override { return 8; }

  void Init(char* state) override {
    *reinterpret_cast<int64_t*>(state) = 0;
  }

  void Push(const FieldValue&, char* state) override {
    ++(*reinterpret_cast<int64_t*>(state));
  }

  void Print(const char* state, std::string* out) const override {
    absl::StrAppend(out, *reinterpret_cast<const int64_t*>(state));
  }
};

class SampleAggregator : public Aggregator {
  size_t StateSize() const override { return 4; }
  void Init(char* state) override {}
  void Push(const FieldValue&, char* state) override {}
  void Print(const char* state, std::string* out) const override {}
};

class Data {
 public:
  Data() {
    agg_.push_back(std::make_unique<CountAggregator>());
    state_offset_.push_back(0);
  }

  void Process(const std::vector<FieldValue>& fields) {
    auto [it, inserted] = rows_.try_emplace(fields[1].AsString());
    char* row = it->second;
    if (inserted) {
      agg_[0]->Init(row);
    }
    agg_[0]->Push(fields[1], row);
  }

  void Render() {
    std::string buffer;
    for (const auto& [key, value] : rows_) {
      buffer.append(key);
      buffer.push_back('\t');
      const char* row = value;
      agg_[0]->Print(row, &buffer);
      buffer.push_back('\n');
      if (buffer.size() > 1 << 15) {
        if (std::fwrite(buffer.data(), 1, buffer.size(), stdout) != buffer.size()) {
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
  using Row = char[8];
  absl::flat_hash_map<std::string, Row> rows_;

  std::vector<size_t> state_offset_;
  std::vector<std::unique_ptr<Aggregator>> agg_;
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

template<class LineFn>
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

int main(int argc, char *argv[]) {
  std::vector<FieldValue> fields;
  Data data;
  ForEachInputLine([&](std::string_view line) {
    SplitLine(line, &fields);
    data.Process(fields);
  });
  data.Render();
  return 0;
}
