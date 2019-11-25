#ifndef GITHUB_ZISZIS_ZG_BASE_INCLUDED
#define GITHUB_ZISZIS_ZG_BASE_INCLUDED

#include <string_view>

// Prints message to cerr and stops process with non-zero code.
void Fail(std::string_view reason);

#endif  // GITHUB_ZISZIS_ZG_BASE_INCLUDED
