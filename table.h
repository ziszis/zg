#ifndef GITHUB_ZISZIS_ZG_TABLE_INCLUDED
#define GITHUB_ZISZIS_ZG_TABLE_INCLUDED

#include "absl/container/flat_hash_map.h"
#include "output.h"
#include "types.h"
#include "varint.h"

class AggregationState {
  template <class Value>
  class NoKey;
  template <class Value>
  class OneKey;
  template <class Value>
  class MultiKey;

 public:
  struct Key {
    Key(int field_, int column_) : field(field_), column(column_) {}
    int field;
    int column;
  };

  struct NoKeyFactory {
    template <class Value>
    NoKey<Value> operator()(Value dflt) const {
      return NoKey<Value>(std::move(dflt));
    }
  };

  struct OneKeyFactory {
    Key key;
    template <class Value>
    OneKey<Value> operator()(Value dflt) const {
      return OneKey<Value>(key, std::move(dflt));
    }
  };

  struct MultiKeyFactory {
    std::vector<Key> key;
    template <class Value>
    MultiKey<Value> operator()(Value dflt) const {
      return MultiKey<Value>(key, std::move(dflt));
    }
  };

 private:
  template <class Value>
  class NoKey {
   public:
    explicit NoKey(Value value) : value_(std::move(value)) {}
    Value& state(const InputRow&) { return value_; }

    template <class Fn>
    void Render(OutputBuffer* out, Fn fn) const {
      fn(value_);
      out->EndLine();
    }

   private:
    Value value_;
  };

  template <class Value>
  class OneKey {
   public:
    OneKey(Key key, Value default_value)
        : key_(key), default_(std::move(default_value)) {}

    Value& state(const InputRow& input) {
      return state_.try_emplace(input[key_.field], default_).first->second;
    }

    template <class Fn>
    void Render(OutputBuffer* out, Fn fn) const {
      for (const auto& [key, value] : state_) {
        out->Column(key_.column)->assign(key);
        fn(value);
        out->EndLine();
      }
    }

   private:
    absl::flat_hash_map<std::string, Value> state_;
    Key key_;
    Value default_;
  };

  template <class Value>
  class MultiKey {
   public:
    MultiKey(std::vector<Key> key, Value default_value)
        : key_(std::move(key)), default_(std::move(default_value)) {}

    Value& state(const InputRow& input) {
      buf_.clear();
      for (const Key& key : key_) {
        std::string_view value = input[key.field];
        if (value.size() > std::numeric_limits<uint32_t>::max()) {
          Fail("Key too long, length=", value.size());
        }
        AppendVarint32(value.size(), &buf_);
        buf_.append(value);
      }
      return state_.try_emplace(buf_, default_).first->second;
    }

    template <class Fn>
    void Render(OutputBuffer* out, Fn fn) const {
      for (const auto& [serialized_key, value] : state_) {
        const char* p = serialized_key.data();
        for (const Key& key : key_) {
          uint32_t len = ParseVarint32(p);
          out->Column(key.column)->assign(p, len);
          p += len;
        }
        fn(value);
        out->EndLine();
      }
    }

   private:
    absl::flat_hash_map<std::string, Value> state_;
    std::vector<Key> key_;
    std::string buf_;
    Value default_;
  };
};

class Table {
 public:
  virtual ~Table() {}
  virtual void PushRow(const InputRow& row) = 0;
  virtual void Render() = 0;
};

#endif  // GITHUB_ZISZIS_ZG_TABLE_INCLUDED
