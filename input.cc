#include "input.h"

#include <cstdio>
#include <string.h>
#include <string>

void ForEachInputLine(const std::function<void(const char*, const char*)>& fn) {
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
        fn(begin, p);
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
          fn(begin, begin + bufferLeft);
        }
        return;
      }
    }
  }
}

void SplitLine(const char* line, const char* end, InputRow* row) {
  row->clear();
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
        row->emplace_back(std::string_view(begin, p - begin));
        return;
      }
      if (*p == ' ' || *p == '\t') {
        row->emplace_back(std::string_view(begin, p - begin));
        begin = p + 1;
        break;
      }
      ++p;
    }
  }
}
