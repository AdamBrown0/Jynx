#include "sema.hh"

#include "ast.hh"
#include "log.hh"
#include "methodtable.hh"
#include "visitor/nameresolver.hh"
#include "visitor/symbolcollector.hh"
#include "visitor/typechecker.hh"

ProgramNode* Sema::analyze(ProgramNode& root) {
  // pass 1: symbol table and scopes
  LOG_DEBUG("Collecting symbols");
  SymbolCollector symbol_collector(ctx);
  symbol_collector.collect(root);
  if (symbol_collector.has_errors()) {
    LOG_ERROR("Symbol collector has failed with {} errors",
              symbol_collector.error_count());
    return nullptr;
  }

  {
    const std::vector<const Type*> no_params;
    const Symbol* main_method =
        ctx.method_table.find_overload("", "main", no_params);

    if (!main_method || !main_method->type ||
        main_method->type != ctx.get_int32_type()) {
      LOG_ERROR("Missing required entry point: int main()");
      return nullptr;
    }
  }

  LOG_DEBUG("Resolving names");
  NameResolver name_resolver(ctx);
  name_resolver.resolve(root);
  if (name_resolver.has_errors()) {
    LOG_ERROR("Name resolver has failed with {} errors",
              name_resolver.error_count());
    return nullptr;
  }

  LOG_DEBUG("Type/decl checking");
  TypeChecker type_checker(ctx);
  type_checker.check(root);

  if (type_checker.has_errors()) {
    LOG_ERROR("Type checker has failed with {} errors",
              type_checker.error_count());
    return nullptr;
  }

  return &root;
}
