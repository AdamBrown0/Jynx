#ifndef SEMA_H_
#define SEMA_H_

#include "ast.hh"
#include "visitor/symbolcollector.hh"
#include "visitor/typechecker.hh"

class Sema {
 public:
  Sema() {}
  ProgramNode<SemaExtra>* analyze(ProgramNode<ParseExtra>&);

 private:
};

#endif  // SEMA_H_
