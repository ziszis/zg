#include "output.h"

#include <cstdio>

#include "base.h"

void OutputBuffer::EndLine() {
  for (std::string& c : columns_) {
    buf_.append(c);
    buf_.push_back('\t');
    c.clear();
  }
  buf_.back() = '\n';
  if (buf_.size() > 1 << 15) Flush();
}

void OutputBuffer::Flush() {
  if (std::fwrite(buf_.data(), 1, buf_.size(), stdout) != buf_.size()) {
    Fail("Write failed");
  }
  buf_.clear();
}
