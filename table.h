#ifndef GITHUB_ZISZIS_ZG_TABLE_INCLUDED
#define GITHUB_ZISZIS_ZG_TABLE_INCLUDED

#include "output.h"
#include "types.h"

class Table {
 public:
  struct Key {
    Key(int field_, int column_) : field(field_), column(column_) {}
    int field;
    int column;
  };

  virtual ~Table() {}
  virtual void PushRow(const InputRow& row) = 0;
  virtual void Render(OutputBuffer* out) const = 0;
};

#endif  // GITHUB_ZISZIS_ZG_TABLE_INCLUDED
