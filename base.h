#ifndef GITHUB_ZISZIS_ZG_BASE_INCLUDED
#define GITHUB_ZISZIS_ZG_BASE_INCLUDED

#include <iostream>
#include <string_view>

// Prints message to cerr and stops process with non-zero code.
template <class... T>
[[noreturn]] void Fail(T&&... args) {
  (std::cerr << ... << std::forward<T>(args)) << std::endl;
  std::exit(1);
}

[[noreturn]] void Unimplemented(std::string_view feature);

[[noreturn]] void LogicError(std::string_view reason = {});

std::string Quoted(std::string_view s);

#endif  // GITHUB_ZISZIS_ZG_BASE_INCLUDED
