#include "sema.hh"
#include "ast.hh"
#include "log.hh"
#include "visitor.hh"

std::unique_ptr<StmtNode<SemaExtra>> Sema::analyze(StmtNode<ParseExtra>& root) {
  // pass 1: symbol table and scops
  

  // pass 2: resolve types

  // pass 3: check types

}