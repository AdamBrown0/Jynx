#ifndef SEMA_H_
#define SEMA_H_

#include "ast.hh"
#include "methodtable.hh"
#include "visitor/visitor.hh"

class Sema {
 public:
  Sema(CompilerContext &ctx) : context(ctx) {}
  ProgramNode<NodeInfo> *analyze(ProgramNode<NodeInfo> &);

 private:
  CompilerContext &context;
};

#endif  // SEMA_H_
