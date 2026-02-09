#ifndef SEMA_H_
#define SEMA_H_

#include "ast.hh"
#include "methodtable.hh"

class Sema {
 public:
  Sema() {}
  ProgramNode<NodeInfo>* analyze(ProgramNode<NodeInfo>&);
    MethodTable &get_method_table() { return methods; }

 private:
    MethodTable methods;
};

#endif  // SEMA_H_
