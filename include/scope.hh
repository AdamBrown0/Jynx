#ifndef SCOPE_H_
#define SCOPE_H_

#include <string>
#include <unordered_map>

#include "symbol.hh"

class Scope {
 public:
  explicit Scope(Scope* parent = nullptr) : parent(parent) {}

  Symbol* declare(const std::string& name, const Type* type,
                  SourceLocation loc);
  Symbol* lookup(const std::string& name, bool walkParent = true);

  Scope* get_parent() const { return parent; }

  void dump() const;

 private:
  Scope* parent = nullptr;
  std::unordered_map<std::string, Symbol*> symbols;
};

#endif  // SCOPE_H_
