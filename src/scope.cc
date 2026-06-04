#include "scope.hh"

#include <iostream>
#include <memory>

Symbol* Scope::declare(const std::string& name, const Type* type,
                       SourceLocation loc) {
  if (symbols.find(name) != symbols.end()) {
    return nullptr;
  }

  auto symbol = std::make_unique<Symbol>();
  symbol->name = name;
  symbol->type = type;
  symbol->location = loc;

  Symbol* sym_ptr = symbol.get();
  symbols[name] = sym_ptr;

  return sym_ptr;
}

Symbol* Scope::lookup(const std::string& name, bool walkParent) {
  Scope* scope = this;
  if (!walkParent) {
    auto it = scope->symbols.find(name);
    return (it != symbols.end()) ? it->second : nullptr;
  }

  while (scope != nullptr) {
    auto it = scope->symbols.find(name);
    if (it != scope->symbols.end()) return it->second;
    scope = scope->parent;
  }

  return nullptr;
}

void Scope::dump() const {
  std::cout << "Scope contents:\n";
  for (const auto& [name, symbol] : symbols) {
    std::cout << "   " << name << " : "
              << (symbol->type ? symbol->type->to_string() : "unknown") << "\n";
  }
}
