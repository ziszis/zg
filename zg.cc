#include <memory>
#include <string>

#include "base.h"
#include "input.h"
#include "spec.h"
#include "types.h"

int main(int argc, char* argv[]) {
  std::vector<std::string> spec;
  for (int i = 1; i < argc; ++i) spec.push_back(argv[i]);

  InputRow row;
  std::unique_ptr<Table> table = ParseSpec(spec);
  ForEachInputLine([&](const char* begin, const char* end) {
    SplitLine(begin, end, &row);
    table->PushRow(row);
  });
  table->Render();
  return 0;
}
