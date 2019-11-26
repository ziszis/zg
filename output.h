#ifndef GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED
#define GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED

#include <string>

class OutputBuffer {
 public:
  std::string* raw() { return &buf_; }

  void MaybeFlush() {
    if (buf_.size() > 1 << 15) Flush();
  }

  ~OutputBuffer() { Flush(); }

 private:
  void Flush();

  std::string buf_;
};

#endif  // GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED
