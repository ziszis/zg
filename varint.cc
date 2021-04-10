#include "varint.h"

#include <bit>
#include <cstring>
#include <limits>

#include "base.h"

namespace internal {
namespace {

template <int k>
inline char* Store(uint32_t x, char* out) {
  static_assert(std::endian::native == std::endian::little);
  std::memcpy(out, &x, k);
  return out + k;
}

template <int k>
inline uint32_t Load(const char*& p) {
  uint32_t x = 0;
  static_assert(std::endian::native == std::endian::little);
  std::memcpy(&x, p, k);
  p += k;
  return x;
}

}  // namespace

char* AppendVarintSlow32(uint32_t x, char* out) {
  if (x < uint32_t{1} << 14) {
    return Store<2>((x << 2) + 1, out);
  } else if (x < uint32_t{1} << 21) {
    return Store<3>((x << 3) + 3, out);
  } else if (x < uint32_t{1} << 28) {
    return Store<4>((x << 4) + 7, out);
  } else {
    *out++ = 0x0f;
    return Store<4>(x, out);
  }
}

uint32_t ParseVarintSlow32(uint8_t lead, const char*& s) {
  if ((lead & 2) == 0) {
    return Load<2>(s) >> 2;
  } else if ((lead & 4) == 0) {
    return Load<3>(s) >> 3;
  } else if ((lead & 8) == 0) {
    return Load<4>(s) >> 4;
  } else {
    ++s;
    return Load<4>(s);
  }
}

}  // internal
