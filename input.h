#ifndef GITHUB_ZISZIS_ZG_INPUT_INCLUDED
#define GITHUB_ZISZIS_ZG_INPUT_INCLUDED

#include <functional>

// Reads stdin, calls `fn` for each line (line terminator is not included in
// fn's arguments).
//
// Note, linefeed character is not specially handled (i.e. is
// passed to `fn` as a regular line byte).
void ForEachInputLine(const std::function<void(const char*, const char*)>& fn);

#endif  // GITHUB_ZISZIS_ZG_INPUT_INCLUDED
