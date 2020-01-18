#include "base.h"

#include "absl/strings/str_cat.h"

std::string Quoted(std::string_view s) { return absl::StrCat("'", s, "'"); }
