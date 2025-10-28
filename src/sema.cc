#include "sema.hh"

#include <cstdlib>

#include "ast.hh"
#include "log.hh"
#include "visitor/symbolcollector.hh"
#include "visitor/treetransformer.hh"
#include "visitor/typechecker.hh"

ProgramNode<SemaExtra>* Sema::analyze(ProgramNode<ParseExtra>& root) {
  // pass 1: symbol table and scopes
  LOG_DEBUG("Collecting symbols");
  SymbolCollectorVisitor symbol_collector;
  root.accept(symbol_collector);
  if (symbol_collector.has_errors()) {
    LOG_ERROR("Symbol collector has failed with {} errors",
              symbol_collector.error_count());
  }

  LOG_DEBUG("Type/decl checking");
  TypeCheckerVisitor type_checker(symbol_collector.get_global_symbols());
  root.accept(type_checker);

  if (type_checker.has_errors()) {
    LOG_ERROR("Type checker has failed with {} errors",
              type_checker.error_count());
    exit(1);
  }

  LOG_DEBUG("Transforming tree");
  TreeTransformer tree_transformer(type_checker.get_expr_types());
  return tree_transformer.transform(root);
}
