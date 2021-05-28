#ifndef GITHUB_ZISZIS_ZG_STORAGE_INCLUDED
#define GITHUB_ZISZIS_ZG_STORAGE_INCLUDED

#include <limits>
#include <variant>

#include "expr.h"
#include "output.h"
#include "types.h"

class DynamicStorage {
 public:
  using Handle = uint32_t;

  Handle Store(std::string_view data);
  void Update(Handle handle, std::string_view new_data);
  std::string_view Load(Handle handle) const;
  void Reset();

 private:
  std::string storage_;
  std::vector<uint64_t> offsets_;
};

class MultiColumnDynamicStorage {
 public:
  using Handle = DynamicStorage::Handle;

  explicit MultiColumnDynamicStorage(
      std::vector<ExprColumn<std::string_view>> columns);

  Handle Store(const InputRow& row);
  void Update(Handle handle, const InputRow& row);
  void Print(Handle handle, OutputTable& out) const;
  void Reset() { stg_.Reset(); }

 private:
  std::string_view Serialize(const InputRow& row) const;

  DynamicStorage stg_;
  std::vector<ExprColumn<std::string_view>> columns_;
  mutable std::string buf_;
};

#endif  // GITHUB_ZISZIS_ZG_STORAGE_INCLUDED
