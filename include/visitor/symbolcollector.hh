#ifndef SYMBOLCOLLECTOR_H_
#define SYMBOLCOLLECTOR_H_

#include "ast.hh"
#include "visitor.hh"

class SymbolCollectorVisitor : public ASTVisitor<NodeInfo> {
 public:
  using ASTVisitor<NodeInfo>::enter;
  using ASTVisitor<NodeInfo>::exit;
  using ASTVisitor<NodeInfo>::before_else;

  std::unordered_map<std::string, Symbol> get_global_symbols() {
    return global_symbols;
  }

  void enter(BlockNode<NodeInfo> &node) override;
  void exit(BlockNode<NodeInfo> &node) override;
  void enter(MethodDeclNode<NodeInfo> &node) override;
  void exit(MethodDeclNode<NodeInfo> &node) override;
  void enter(ConstructorDeclNode<NodeInfo> &node) override;
  void exit(ConstructorDeclNode<NodeInfo> &node) override;

  void visit(ParamNode<NodeInfo> &node) override;
  void visit(VarDeclNode<NodeInfo> &node) override;
  void visit(ClassNode<NodeInfo> &node) override;
  void visit(FieldDeclNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(ConstructorDeclNode<NodeInfo> &node) override;
};

#endif  // SYMBOLCOLLECTOR_H_
