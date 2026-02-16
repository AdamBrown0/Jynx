#include "sema.hh"

#include <unordered_map>

#include "ast.hh"
#include "log.hh"
#include "methodtable.hh"
#include "visitor/nameresolver.hh"
#include "visitor/symbolcollector.hh"
#include "visitor/typechecker.hh"

ProgramNode<NodeInfo>* Sema::analyze(ProgramNode<NodeInfo>& root) {
  // pass 1: symbol table and scopes
  LOG_DEBUG("Collecting symbols");
  SymbolCollectorVisitor symbol_collector(context);
  root.accept(symbol_collector);
  if (symbol_collector.has_errors()) {
    LOG_ERROR("Symbol collector has failed with {} errors",
              symbol_collector.error_count());
    return nullptr;
  }

  {
    const std::vector<TokenType> no_params;
    const Symbol* main_method =
        context.method_table.find_overload("global", "main", no_params);
    if (!main_method || main_method->type != TokenType::TOKEN_DATA_TYPE ||
        main_method->type_name != "int") {
      LOG_ERROR("Missing required entry point: int main() with no parameters");
      return nullptr;
    }
  }

  LOG_DEBUG("Resolving names");
  NameResolver name_resolver(context);
  root.accept(name_resolver);
  if (name_resolver.has_errors()) {
    LOG_ERROR("Name resolver has failed with {} errors",
              name_resolver.error_count());
    return nullptr;
  }

  LOG_DEBUG("Type/decl checking");
  TypeCheckerVisitor type_checker(context);
  root.accept(type_checker);

  if (type_checker.has_errors()) {
    LOG_ERROR("Type checker has failed with {} errors",
              type_checker.error_count());
    return nullptr;
  }

  return &root;
}
