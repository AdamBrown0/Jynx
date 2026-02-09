#ifndef NAMERESOLVER_H_
#define NAMERESOLVER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast.hh"
#include "methodtable.hh"
#include "visitor/visitor.hh"

class NameResolver : public ASTVisitor<NodeInfo> {
 public:
  NameResolver(std::unordered_map<std::string, Symbol> &symbols,
               MethodTable &methods) {
    set_global_symbols(&symbols);
    set_method_table(&methods);
  }

  void enter(BlockNode<NodeInfo> &node) override;
  void exit(BlockNode<NodeInfo> &node) override;
  void enter(ProgramNode<NodeInfo> &node) override;
  void exit(ProgramNode<NodeInfo> &node) override;
  void enter(MethodDeclNode<NodeInfo> &node) override;
  void exit(MethodDeclNode<NodeInfo> &node) override;
  void enter(ConstructorDeclNode<NodeInfo> &node) override;
  void exit(ConstructorDeclNode<NodeInfo> &node) override;
  void enter(ClassNode<NodeInfo> &node) override;
  void exit(ClassNode<NodeInfo> &node) override;

  void visit(VarDeclNode<NodeInfo> &node) override;
  void visit(ParamNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(IdentifierExprNode<NodeInfo> &node) override;
  void visit(ClassNode<NodeInfo> &node) override;
  void visit(MethodCallNode<NodeInfo> &node) override;
  void visit(AssignmentExprNode<NodeInfo> &node) override;

 private:
  Symbol *find_method_overload(const std::string &owner,
                               const std::string &name,
                               const std::vector<TokenType> &param_types);
};

#endif  // NAMERESOLVER_H_
