#include "base.h"

#include <cstdlib>
#include <iostream>

void Fail(std::string_view reason) {
  std::cerr << reason << std::endl;
  std::abort();
}
