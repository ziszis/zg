#ifndef GITHUB_ZISZIS_ZG_INPUT_INCLUDED
#define GITHUB_ZISZIS_ZG_INPUT_INCLUDED

#include <functional>
#include <string>

#include "types.h"

// Reads stdin, calls `fn` for each line (line terminator is not included in
// fn's arguments).
//
// Note, linefeed character is not specially handled (i.e. is
// passed to `fn` as a regular line byte).
void ForEachInputLine(const std::function<void(const char*, const char*)>& fn);

// Splits [line, end) into fields, separated by one or more of tabs/spaces.
// Existing `row` contents are cleared.
void SplitLine(const char* line, const char* end, InputRow* row);

#endif  // GITHUB_ZISZIS_ZG_INPUT_INCLUDED
