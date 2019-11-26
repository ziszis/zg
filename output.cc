#include "output.h"

#include <cstdio>

#include "base.h"

void OutputBuffer::Flush() {
  if (std::fwrite(buf_.data(), 1, buf_.size(), stdout) != buf_.size()) {
    Fail("Write failed");
  }
  buf_.clear();
}
