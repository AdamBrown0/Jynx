#ifndef VISITOR_H_
#define VISITOR_H_

#include <unordered_map>

#include "ast.hh"

template <typename Extra>
class ASTVisitor {
 public:
  virtual void visit(ASTNode<Extra>& node) = 0;
  virtual void visit(BinaryExprNode<Extra>& node) = 0;
  virtual void visit(UnaryExprNode<Extra>& node) = 0;
  virtual void visit(LiteralExprNode<Extra>& node) = 0;
  virtual void visit(IdentifierExprNode<Extra>& node) = 0;
  virtual void visit(AssignmentExprNode<Extra>& node) = 0;
  virtual void visit(MethodCallNode<Extra>& node) = 0;
  virtual void visit(ArgumentNode<Extra>& node) = 0;
  virtual void visit(ParamNode<Extra>& node) = 0;
  virtual void visit(ProgramNode<Extra>& node) = 0;
  virtual void visit(BlockNode<Extra>& node) = 0;
  virtual void visit(VarDeclNode<Extra>& node) = 0;
  virtual void visit(IfStmtNode<Extra>& node) = 0;
  virtual void visit(ReturnStmtNode<Extra>& node) = 0;
  virtual void visit(ClassNode<Extra>& node) = 0;
  virtual void visit(FieldDeclNode<Extra>& node) = 0;
  virtual void visit(MethodDeclNode<Extra>& node) = 0;
  virtual void visit(ConstructorDeclNode<Extra>& node) = 0;
};

class SymbolCollectorVisitor : public ASTVisitor<ParseExtra> {};

class DeclarationCheckerVisitor {
 public:
  virtual void visit(ASTNode<ParseExtra>& node) = 0;
  virtual void visit(BinaryExprNode<ParseExtra>& node) = 0;
  virtual void visit(UnaryExprNode<ParseExtra>& node) = 0;
  virtual void visit(LiteralExprNode<ParseExtra>& node) = 0;
  virtual void visit(IdentifierExprNode<ParseExtra>& node) = 0;
  virtual void visit(AssignmentExprNode<ParseExtra>& node) = 0;
  virtual void visit(MethodCallNode<ParseExtra>& node) = 0;
  virtual void visit(ArgumentNode<ParseExtra>& node) = 0;
  virtual void visit(ParamNode<ParseExtra>& node) = 0;
  virtual void visit(ProgramNode<ParseExtra>& node) = 0;
  virtual void visit(BlockNode<ParseExtra>& node) = 0;
  virtual void visit(VarDeclNode<ParseExtra>& node) = 0;
  virtual void visit(IfStmtNode<ParseExtra>& node) = 0;
  virtual void visit(ReturnStmtNode<ParseExtra>& node) = 0;
  virtual void visit(ClassNode<ParseExtra>& node) = 0;
  virtual void visit(FieldDeclNode<ParseExtra>& node) = 0;
  virtual void visit(MethodDeclNode<ParseExtra>& node) = 0;
  virtual void visit(ConstructorDeclNode<ParseExtra>& node) = 0;

  virtual ~DeclarationCheckerVisitor() = default;

 private:
  std::unordered_map<std::string, TokenType> symbol_table;
};

class TypeCheckerVisitor {
 public:
  virtual void visit(ASTNode<ParseExtra>& node) = 0;
  virtual void visit(BinaryExprNode<ParseExtra>& node) = 0;
  virtual void visit(UnaryExprNode<ParseExtra>& node) = 0;
  virtual void visit(LiteralExprNode<ParseExtra>& node) = 0;
  virtual void visit(IdentifierExprNode<ParseExtra>& node) = 0;
  virtual void visit(AssignmentExprNode<ParseExtra>& node) = 0;
  virtual void visit(MethodCallNode<ParseExtra>& node) = 0;
  virtual void visit(ArgumentNode<ParseExtra>& node) = 0;
  virtual void visit(ParamNode<ParseExtra>& node) = 0;
  virtual void visit(ProgramNode<ParseExtra>& node) = 0;
  virtual void visit(BlockNode<ParseExtra>& node) = 0;
  virtual void visit(VarDeclNode<ParseExtra>& node) = 0;
  virtual void visit(IfStmtNode<ParseExtra>& node) = 0;
  virtual void visit(ReturnStmtNode<ParseExtra>& node) = 0;
  virtual void visit(ClassNode<ParseExtra>& node) = 0;
  virtual void visit(FieldDeclNode<ParseExtra>& node) = 0;
  virtual void visit(MethodDeclNode<ParseExtra>& node) = 0;
  virtual void visit(ConstructorDeclNode<ParseExtra>& node) = 0;

  virtual ~TypeCheckerVisitor() = default;
};

#endif  // VISITOR_H_
