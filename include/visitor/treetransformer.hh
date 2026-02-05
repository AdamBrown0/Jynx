#ifndef TREETRANSFORMER_H_
#define TREETRANSFORMER_H_

#include <stack>
#include <unordered_map>

#include "ast.hh"
#include "visitor/visitor.hh"
#include "visitor/typechecker.hh"

class TreeTransformer : public ASTVisitor<NodeInfo> {
 public:
  using ASTVisitor<NodeInfo>::enter;
  using ASTVisitor<NodeInfo>::exit;
  using ASTVisitor<NodeInfo>::before_else;

  TreeTransformer(
      const std::unordered_map<ASTNode<NodeInfo> *, TypeInfo> &expr_types)
      : expr_types(expr_types) {}  // Copy the map

  ProgramNode<NodeInfo> *transform(ProgramNode<NodeInfo> &node);

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
  void visit(WhileStmtNode<NodeInfo> &node) override;
  void visit(ReturnStmtNode<NodeInfo> &node) override;
  void visit(ClassNode<NodeInfo> &node) override;
  void visit(FieldDeclNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(ConstructorDeclNode<NodeInfo> &node) override;
  void visit(ExprStmtNode<NodeInfo> &node) override;

  void exit(IfStmtNode<NodeInfo> &node) override;
  void exit(WhileStmtNode<NodeInfo> &node) override;

 private:
  std::unordered_map<ASTNode<NodeInfo> *, TypeInfo>
      expr_types;  // Own the copy

  std::stack<StmtNode<NodeInfo> *> stmt_stack;
  std::stack<ExprNode<NodeInfo> *> expr_stack;

  TypeInfo lookupType(ASTNode<NodeInfo> *node) {
    auto it = expr_types.find(node);
    if (it != expr_types.end()) {
      return it->second;
    }
    return {TokenType::TOKEN_UNKNOWN, ""};
  }
};

#endif  // TREETRANSFORMER_H_
