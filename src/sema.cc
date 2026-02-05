#include "sema.hh"

#include "ast.hh"
#include "log.hh"
#include "visitor/symbolcollector.hh"
#include "visitor/typechecker.hh"

ProgramNode<NodeInfo>* Sema::analyze(ProgramNode<NodeInfo>& root) {
  // pass 1: symbol table and scopes
  LOG_DEBUG("Collecting symbols");
  SymbolCollectorVisitor symbol_collector;
  root.accept(symbol_collector);
  if (symbol_collector.has_errors()) {
    LOG_ERROR("Symbol collector has failed with {} errors",
              symbol_collector.error_count());
    return nullptr;
  }

  LOG_DEBUG("Type/decl checking");
  TypeCheckerVisitor type_checker(symbol_collector.get_global_symbols());
  root.accept(type_checker);

  if (type_checker.has_errors()) {
    LOG_ERROR("Type checker has failed with {} errors",
              type_checker.error_count());
    return nullptr;
  }

  return &root;
}
