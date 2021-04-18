#include <memory>
#include <string>

#include "base.h"
#include "input.h"
#include "pipeline.h"
#include "spec-parser.h"
#include "spec.h"
#include "types.h"

int main(int argc, char* argv[]) {
  std::string spec_str;
  for (int i = 1; i < argc; ++i) {
    if (i != 1) spec_str.push_back(' ');
    spec_str.append(argv[i]);
  }
  spec::Pipeline spec = spec::Parse(spec_str);

  std::unique_ptr<Table> table = BuildPipeline(spec);

  InputRow row;
  ForEachInputLine([&](const char* begin, const char* end) {
    row.Reset(std::string_view(begin, end - begin));
    table->PushRow(row);
  });
  table->Finish();

  return 0;
}
