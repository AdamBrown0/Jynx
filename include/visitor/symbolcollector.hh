#ifndef SYMBOLCOLLECTOR_H_
#define SYMBOLCOLLECTOR_H_

#include "ast.hh"
#include "visitor.hh"

class SymbolCollectorVisitor : public ASTVisitor<NodeInfo> {
 public:
  std::unordered_map<std::string, Symbol> get_global_symbols() {
    return global_symbols;
  }

  void visit(ASTNode<NodeInfo> &node) override;
  void visit(BinaryExprNode<NodeInfo> &node) override;
  void visit(UnaryExprNode<NodeInfo> &node) override;
  void visit(LiteralExprNode<NodeInfo> &node) override;
  void visit(IdentifierExprNode<NodeInfo> &node) override;
  void visit(AssignmentExprNode<NodeInfo> &node) override;
  void visit(MethodCallNode<NodeInfo> &node) override;
  void visit(ArgumentNode<NodeInfo> &node) override;
  void visit(ParamNode<NodeInfo> &node) override;
  void visit(ProgramNode<NodeInfo> &node) override;
  void visit(BlockNode<NodeInfo> &node) override;
  void visit(VarDeclNode<NodeInfo> &node) override;
  void visit(IfStmtNode<NodeInfo> &node) override;
  void visit(WhileStmtNode<NodeInfo> &) override {};
  void visit(ReturnStmtNode<NodeInfo> &node) override;
  void visit(ClassNode<NodeInfo> &node) override;
  void visit(FieldDeclNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(ConstructorDeclNode<NodeInfo> &node) override;
  void visit(ExprStmtNode<NodeInfo> &node) override;
};

#endif  // SYMBOLCOLLECTOR_H_
