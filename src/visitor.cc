#include "visitor/visitor.hh"

#include "ast.hh"
#include "visitor/symbolcollector.hh"
#include "visitor/typechecker.hh"

// ============================================================================
// SymbolCollectorVisitor stub implementations
// ============================================================================
void SymbolCollectorVisitor::visit(ASTNode<ParseExtra>& node) {
  ASTVisitor<ParseExtra>::visit(node);
}
// void SymbolCollectorVisitor::visit(BinaryExprNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(UnaryExprNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(LiteralExprNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(IdentifierExprNode<ParseExtra>& node) {}
// void SymbolCollectorVisitor::visit(AssignmentExprNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(MethodCallNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(ArgumentNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(ParamNode<ParseExtra>& node) {}
// void SymbolCollectorVisitor::visit(ProgramNode<ParseExtra>& node) {}
// void SymbolCollectorVisitor::visit(BlockNode<ParseExtra>& node) {}
// void SymbolCollectorVisitor::visit(VarDeclNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(IfStmtNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(ReturnStmtNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(ClassNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(FieldDeclNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(MethodDeclNode<ParseExtra>& node) {}
void SymbolCollectorVisitor::visit(ConstructorDeclNode<ParseExtra>& node) {}
// void SymbolCollectorVisitor::visit(ExprStmtNode<ParseExtra>& node) {}

// ============================================================================
// TypeCheckerVisitor stub implementations
// ============================================================================
void TypeCheckerVisitor::visit(ASTNode<ParseExtra>& node) {
  ASTVisitor<ParseExtra>::visit(node);
}
// void TypeCheckerVisitor::visit(BinaryExprNode<ParseExtra>& node) {}
// void TypeCheckerVisitor::visit(UnaryExprNode<ParseExtra>& node) {}
// void TypeCheckerVisitor::visit(LiteralExprNode<ParseExtra>& node) {}
// void TypeCheckerVisitor::visit(IdentifierExprNode<ParseExtra>& node) {}
// void TypeCheckerVisitor::visit(AssignmentExprNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(MethodCallNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(ArgumentNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(ParamNode<ParseExtra>& node) {}
// void TypeCheckerVisitor::visit(ProgramNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(BlockNode<ParseExtra>& node) {}
// void TypeCheckerVisitor::visit(VarDeclNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(IfStmtNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(ReturnStmtNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(ClassNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(FieldDeclNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(MethodDeclNode<ParseExtra>& node) {}
void TypeCheckerVisitor::visit(ConstructorDeclNode<ParseExtra>& node) {}
// void TypeCheckerVisitor::visit(ExprStmtNode<ParseExtra>& node) {}
