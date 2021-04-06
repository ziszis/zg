#ifndef GITHUB_ZISZIS_ZG_VARINT_INCLUDED
#define GITHUB_ZISZIS_ZG_VARINT_INCLUDED

#include <string>

namespace internal {
char* AppendVarintSlow32(uint32_t x, char* out);
uint32_t ParseVarintSlow32(uint8_t lead, const char*& s);
}  // internal

constexpr int kVarint32MaxLength = 5;

inline char* AppendVarint32(uint32_t x, char* out) {
  if (__builtin_expect(x < 128, 1)) {
    *out = x << 1;
    return out + 1;
  } else {
    return internal::AppendVarintSlow32(x, out);
  }
}

// Convenience wrapper to append to a string. Several times slower than the
// version above.
inline void AppendVarint32(uint32_t x, std::string* out) {
  char buf[kVarint32MaxLength];
  out->append(buf, AppendVarint32(x, buf) - buf);
}

inline uint32_t ParseVarint32(const char*& s) {
  uint8_t lead = *s;
  if (__builtin_expect((lead & 1) == 0, 1)) {
    ++s;
    return lead >> 1;
  } else {
    return internal::ParseVarintSlow32(lead, s);
  }
}

#endif  // GITHUB_ZISZIS_ZG_VARINT_INCLUDED
