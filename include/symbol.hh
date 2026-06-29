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

  int field_count;  // param for functions, field for structs
  std::vector<Symbol*> fields;

  std::string owner_class;

  virtual ~Symbol() = default;
};

struct VariableSymbol final : Symbol {
  bool is_mutable = true;
};

struct FunctionSymbol final : Symbol {
  std::vector<const Type*> param_types;
  std::vector<FunctionSymbol*> overloads;
};

class Scope;

struct ClassSymbol final : Symbol {
  Scope* member_scope = nullptr;
};

#endif  // SYMBOL_H_
