#ifndef SEMA_H_
#define SEMA_H_

#include "ast.hh"
#include "visitor/visitor.hh"

class Sema {
 public:
  Sema(CompilerContext& ctx) : ctx(ctx) {}
  ProgramNode* analyze(ProgramNode&);

 private:
  CompilerContext& ctx;
};

#endif  // SEMA_H_
