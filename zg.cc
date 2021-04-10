#include <memory>
#include <string>

#include "base.h"
#include "input.h"
#include "spec.h"
#include "types.h"

int main(int argc, char* argv[]) {
  std::vector<std::string> spec;
  for (int i = 1; i < argc; ++i) spec.push_back(argv[i]);

  InputRow row("");
  std::unique_ptr<Table> table = ParseSpec(spec);
  ForEachInputLine([&](const char* begin, const char* end) {
    row.Reset(std::string_view(begin, end - begin));
    table->PushRow(row);
  });
  OutputBuffer out;
  table->Render(&out);
  return 0;
}
