#ifndef GITHUB_ZISZIS_ZG_TABLE_INCLUDED
#define GITHUB_ZISZIS_ZG_TABLE_INCLUDED

#include "absl/container/flat_hash_map.h"
#include "output.h"
#include "types.h"
#include "varint.h"

class AggregationState {
 public:
  struct Key {
    Key(int field_, int column_) : field(field_), column(column_) {}
    int field;
    int column;
  };

  template <class Value>
  class NoKeys {
   public:
    template <class InsertFn, class UpdateFn>
    void Push(const InputRow& row, InsertFn insert, UpdateFn update) {
      if (value_) {
        update(*value_);
      } else {
        value_ = insert();
      }
    }

    template <class Fn>
    void Render(OutputBuffer* out, Fn fn) const {
      if (value_) {
        fn(*value_);
        out->EndLine();
      }
    }

   private:
    absl::optional<Value> value_;
  };

  template <class Value>
  class SingleKey {
   public:
    explicit SingleKey(const Key& key) : key_(key) {}

    template <class InsertFn, class UpdateFn>
    void Push(const InputRow& row, InsertFn insert, UpdateFn update) {
      auto it = state_.find(row[key_.field]);
      if (it == state_.end()) {
        state_.emplace_hint(it, row[key_.field], insert());
      } else {
        update(it->second);
      }
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
  };

  template <class Value>
  class CompositeKey {
   public:
    explicit CompositeKey(std::vector<Key> key) : key_(key) {}

    template <class InsertFn, class UpdateFn>
    void Push(const InputRow& row, InsertFn insert, UpdateFn update) {
      buf_.clear();
      for (const auto& key : key_) {
        std::string_view value = row[key.field];
        if (value.size() > std::numeric_limits<uint32_t>::max()) {
          Fail("Key too long, length=", value.size());
        }
        AppendVarint32(value.size(), &buf_);
        buf_.append(value);
      }

      auto it = state_.find(buf_);
      if (it == state_.end()) {
        state_.emplace_hint(it, buf_, insert());
      } else {
        update(it->second);
      }
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
  };
};

class Table {
 public:
  virtual ~Table() {}
  virtual void PushRow(const InputRow& row) = 0;
  virtual void Render() = 0;
};

#endif  // GITHUB_ZISZIS_ZG_TABLE_INCLUDED
