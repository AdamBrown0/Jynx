#ifndef SYMBOLCOLLECTOR_H_
#define SYMBOLCOLLECTOR_H_

#include "ast.hh"
#include "methodtable.hh"
#include "visitor.hh"

class SymbolCollectorVisitor : public ASTVisitor<NodeInfo> {
 public:
  SymbolCollectorVisitor(CompilerContext &ctx) : ASTVisitor<NodeInfo>(ctx) {}

  void enter(BlockNode<NodeInfo> &node) override;
  void exit(BlockNode<NodeInfo> &node) override;
  void enter(MethodDeclNode<NodeInfo> &node) override;
  void exit(MethodDeclNode<NodeInfo> &node) override;
  void enter(ConstructorDeclNode<NodeInfo> &node) override;
  void exit(ConstructorDeclNode<NodeInfo> &node) override;

  void visit(ParamNode<NodeInfo> &node) override;
  void visit(ArgumentNode<NodeInfo> &node) override;
  void visit(LiteralExprNode<NodeInfo> &node) override;
  void visit(VarDeclNode<NodeInfo> &node) override;
  void visit(ClassNode<NodeInfo> &node) override;
  void visit(FieldDeclNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(ConstructorDeclNode<NodeInfo> &node) override;

 private:
  TokenType resolve_param_type(const std::string &name);
};

#endif  // SYMBOLCOLLECTOR_H_
