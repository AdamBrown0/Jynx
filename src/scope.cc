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

  Symbol* raw = symbol.get();
  symbols[name] = std::move(symbol);
  return raw;
}

VariableSymbol* Scope::declare(const std::string& name, const Type* type,
                               bool is_mutable, SourceLocation loc) {
  if (symbols.find(name) != symbols.end()) return nullptr;

  auto var = std::make_unique<VariableSymbol>();
  var->name = name;
  var->type = type;
  var->location = loc;
  var->is_mutable = is_mutable;

  VariableSymbol* ptr = var.get();
  symbols[name] = std::move(var);
  return ptr;
}

FunctionSymbol* Scope::declare(const std::string& name, const Type* return_type,
                               const std::vector<const Type*> param_types,
                               SourceLocation loc) {
  if (symbols.find(name) != symbols.end()) {
    return nullptr;
  }

  auto func = std::make_unique<FunctionSymbol>();
  func->name = name;
  func->type = return_type;
  func->param_types = param_types;
  func->location = loc;

  FunctionSymbol* ptr = func.get();
  symbols[name] = std::move(func);
  return ptr;
}

Symbol* Scope::lookup(const std::string& name, bool walkParent) {
  Scope* scope = this;
  if (!walkParent) {
    auto it = scope->symbols.find(name);
    return (it != symbols.end()) ? it->second.get() : nullptr;
  }

  while (scope != nullptr) {
    auto it = scope->symbols.find(name);
    if (it != scope->symbols.end()) return it->second.get();
    scope = scope->parent;
  }

  return nullptr;
}

Symbol* Scope::lookup(const std::string& name, Scope* startingScope,
                      bool walkParent) {
  Scope* scope = startingScope;
  while (scope) {
    Symbol* sym = scope->lookup(name, walkParent);
    if (sym) return sym;

    if (walkParent) scope = scope->get_parent();
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
