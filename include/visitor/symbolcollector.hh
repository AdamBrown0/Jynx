#ifndef SYMBOLCOLLECTOR_H_
#define SYMBOLCOLLECTOR_H_

#include "ast.hh"
#include "visitor.hh"

class SymbolCollectorVisitor : public ASTVisitor<ParseExtra> {
 public:
  std::unordered_map<std::string, Symbol> get_global_symbols() {
    return global_symbols;
  }

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
  void visit(ExprStmtNode<ParseExtra> &node) override;
};

#endif  // SYMBOLCOLLECTOR_H_
