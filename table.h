#ifndef GITHUB_ZISZIS_ZG_TABLE_INCLUDED
#define GITHUB_ZISZIS_ZG_TABLE_INCLUDED

#include "absl/container/flat_hash_map.h"
#include "output.h"
#include "types.h"

class AggregationState {
  template <class Value> class NoKey;
  template <class Value> class OneKey;

 public:
  struct NoKeyFactory {
    template <class Value>
    NoKey<Value> operator()(Value dflt) const {
      return NoKey<Value>(std::move(dflt));
    }
  };

  struct OneKeyFactory {
    int key_field;
    template <class Value>
    OneKey<Value> operator()(Value dflt) const {
      return OneKey<Value>(key_field, std::move(dflt));
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
      out->raw()->push_back('\n');
    }

   private:
    Value value_;
  };

  template <class Value>
  class OneKey {
   public:
    OneKey(int key_field, Value default_value)
        : key_field_(key_field), default_(std::move(default_value)) {}

    Value& state(const InputRow& input) {
      return state_.try_emplace(input[key_field_].AsString(), default_)
          .first->second;
    }

    template <class Fn>
    void Render(OutputBuffer* out, Fn fn) const {
      for (const auto& [key, value] : state_) {
        out->raw()->append(key);
        out->raw()->push_back('\t');
        fn(value);
        out->raw()->push_back('\n');
        out->MaybeFlush();
      }
    }

   private:
    absl::flat_hash_map<std::string, Value> state_;
    int key_field_;
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
