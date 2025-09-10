#ifndef SEMA_H_
#define SEMA_H_

#include <unordered_map>
#include "ast.hh"
#include "visitor.hh"

struct Symbol {

};

class Sema {
 public:
  Sema() {}
  std::unique_ptr<StmtNode<SemaExtra>> analyze(StmtNode<ParseExtra>&);

 private:

};

#endif  // SEMA_H_
