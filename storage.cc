#include "storage.h"

#include "varint.h"

// TODO(zis): Make handles 8 bytes and implement SSO for small values?

DynamicStorage::Handle DynamicStorage::Store(std::string_view data) {
  Handle result = offsets_.size();
  offsets_.push_back(storage_.size());
  AppendVarint32(data.size(), &storage_);
  storage_.append(data);
  return result;
}

void DynamicStorage::Update(DynamicStorage::Handle handle,
                            std::string_view new_data) {
  char *curr = storage_.data() + offsets_[handle];
  const char* p = curr;
  uint32_t len = ParseVarint32(p);
  if (new_data.size() > len) {
    // TODO(zis): Defragment storage if utilization drops too low?
    offsets_[handle] = storage_.size();
    AppendVarint32(new_data.size(), &storage_);
    storage_.append(new_data);
  } else {
    curr = AppendVarint32(new_data.size(), curr);
    std::memcpy(curr, new_data.data(), new_data.size());
  }
}

std::string_view DynamicStorage::Load(DynamicStorage::Handle handle) const {
  const char* p = storage_.data() + offsets_[handle];
  uint32_t len = ParseVarint32(p);
  return {p, len};
}

void DynamicStorage::Reset() {
  decltype(storage_)().swap(storage_);
  decltype(offsets_)().swap(offsets_);
}

MultiColumnDynamicStorage::MultiColumnDynamicStorage(
    std::vector<ExprColumn<std::string_view>> columns)
    : columns_(std::move(columns)) {}

MultiColumnDynamicStorage::Handle MultiColumnDynamicStorage::Store(
    const InputRow& row) {
  return stg_.Store(Serialize(row));
}

void MultiColumnDynamicStorage::Update(Handle handle, const InputRow& row) {
  stg_.Update(handle, Serialize(row));
}

void MultiColumnDynamicStorage::Print(Handle handle, OutputTable& out) const {
  std::string_view value = stg_.Load(handle);
  if (columns_.size() == 1) {
    columns_[0].Print(value, out);
  } else {
    const char* p = value.data();
    for (int i = 0; i < columns_.size(); ++i) {
      uint32_t len = (i + 1 != columns_.size())
                         ? ParseVarint32(p)
                         : value.size() - (p - value.data());
      columns_[i].Print(std::string_view(p, len), out);
      p += len;
    }
  }
}

std::string_view MultiColumnDynamicStorage::Serialize(
    const InputRow& row) const {
  if (columns_.size() == 1) {
    return columns_[0].Eval(row);
  } else {
    buf_.clear();
    for (int i = 0; i < columns_.size(); ++i) {
      std::string_view field = columns_[i].Eval(row);
      if (i + 1 != columns_.size()) {
        AppendVarint32(field.size(), &buf_);
      }
      buf_.append(field);
    }
    return buf_;
  }
}
