#include "base.h"

#include "absl/strings/str_cat.h"

[[noreturn]] void Unimplemented(std::string_view feature) {
  Fail(feature, " not implemented");
}

[[noreturn]] void LogicError(std::string_view reason) {
  if (reason.data()) {
    Fail("Should never happen™ (", reason, ")");
  } else {
    Fail("Should never happen™");
  }
}

std::string Quoted(std::string_view s) { return absl::StrCat("'", s, "'"); }
