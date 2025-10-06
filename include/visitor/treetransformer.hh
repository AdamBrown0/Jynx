#ifndef TREETRANSFORMER_H_
#define TREETRANSFORMER_H_

#include <stack>
#include <unordered_map>

#include "ast.hh"
#include "visitor/visitor.hh"

class TreeTransformer : ASTVisitor<ParseExtra> {
 public:
  TreeTransformer(
      const std::unordered_map<ASTNode<ParseExtra> *, TokenType> &expr_types)
      : expr_types(expr_types) {}  // Copy the map

  ProgramNode<SemaExtra> *transform(ProgramNode<ParseExtra> &node);

  void visit(ASTNode<ParseExtra> &node) override;
  void visit(BinaryExprNode<ParseExtra> &node) override;
  void visit(UnaryExprNode<ParseExtra> &node) override;
  void visit(LiteralExprNode<ParseExtra> &node) override;
  void visit(IdentifierExprNode<ParseExtra> &node) override;
  void visit(AssignmentExprNode<ParseExtra> &node) override;
  void visit(MethodCallNode<ParseExtra> &node) override;
  void visit(ArgumentNode<ParseExtra> &node) override;
  void visit(ParamNode<ParseExtra> &node) override;
  void visit(ProgramNode<ParseExtra> &node) override;
  void visit(BlockNode<ParseExtra> &node) override;
  void visit(VarDeclNode<ParseExtra> &node) override;
  void visit(IfStmtNode<ParseExtra> &node) override;
  void visit(ReturnStmtNode<ParseExtra> &node) override;
  void visit(ClassNode<ParseExtra> &node) override;
  void visit(FieldDeclNode<ParseExtra> &node) override;
  void visit(MethodDeclNode<ParseExtra> &node) override;
  void visit(ConstructorDeclNode<ParseExtra> &node) override;

 private:
  std::unordered_map<ASTNode<ParseExtra> *, TokenType> expr_types;  // Own the copy

  std::stack<StmtNode<SemaExtra> *> stmt_stack;
  std::stack<ExprNode<SemaExtra> *> expr_stack;

  TokenType lookupType(ASTNode<ParseExtra> *node) {
    auto it = expr_types.find(node);
    return (it != expr_types.end()) ? it->second : TokenType::TOKEN_UNKNOWN;
  }
};

#endif  // TREETRANSFORMER_H_
