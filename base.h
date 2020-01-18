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

std::string Quoted(std::string_view s);

#endif  // GITHUB_ZISZIS_ZG_BASE_INCLUDED
