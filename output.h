#ifndef GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED
#define GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED

#include <string>
#include <vector>

class OutputBuffer {
 public:
  std::string* Column(int i) {
    if (i >= columns_.size()) columns_.resize(i + 1);
    return &columns_[i];
  }

  void EndLine();

  ~OutputBuffer() { Flush(); }

 private:
  void Flush();

  std::vector<std::string> columns_;
  std::string buf_;
};

#endif  // GITHUB_ZISZIS_ZG_OUTPUT_INCLUDED
