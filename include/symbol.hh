#ifndef SYMBOL_H_
#define SYMBOL_H_

#include <string>
#include <vector>

#include "sourcelocation.hh"
#include "type.hh"

struct Symbol {
  std::string name;
  const Type* type = nullptr;
  SourceLocation location;
};

#endif  // SYMBOL_H_
