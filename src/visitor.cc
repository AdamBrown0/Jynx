#include "visitor/visitor.hh"

#include "ast.hh"
#include "visitor/symbolcollector.hh"
#include "visitor/typechecker.hh"

// ============================================================================
// SymbolCollectorVisitor stub implementations
// ============================================================================
void SymbolCollectorVisitor::visit(ASTNode<NodeInfo>& node) {
  ASTVisitor<NodeInfo>::visit(node);
}
// void SymbolCollectorVisitor::visit(BinaryExprNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(UnaryExprNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(LiteralExprNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(IdentifierExprNode<NodeInfo>& node) {}
// void SymbolCollectorVisitor::visit(AssignmentExprNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(MethodCallNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(ArgumentNode<NodeInfo>& node) {}
// void SymbolCollectorVisitor::visit(ParamNode<NodeInfo>& node) {}
// void SymbolCollectorVisitor::visit(ProgramNode<NodeInfo>& node) {}
// void SymbolCollectorVisitor::visit(BlockNode<NodeInfo>& node) {}
// void SymbolCollectorVisitor::visit(VarDeclNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(IfStmtNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(ReturnStmtNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(ClassNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(FieldDeclNode<NodeInfo>& node) {}
// void SymbolCollectorVisitor::visit(MethodDeclNode<NodeInfo>& node) {}
void SymbolCollectorVisitor::visit(ConstructorDeclNode<NodeInfo>& node) {}
// void SymbolCollectorVisitor::visit(ExprStmtNode<NodeInfo>& node) {}

// ============================================================================
// TypeCheckerVisitor stub implementations
// ============================================================================
// void TypeCheckerVisitor::visit(BinaryExprNode<NodeInfo>& node) {}
// void TypeCheckerVisitor::visit(UnaryExprNode<NodeInfo>& node) {}
// void TypeCheckerVisitor::visit(LiteralExprNode<NodeInfo>& node) {}
// void TypeCheckerVisitor::visit(IdentifierExprNode<NodeInfo>& node) {}
// void TypeCheckerVisitor::visit(AssignmentExprNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(MethodCallNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(ArgumentNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(ParamNode<NodeInfo>& node) {}
// void TypeCheckerVisitor::visit(ProgramNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(BlockNode<NodeInfo>& node) {}
// void TypeCheckerVisitor::visit(VarDeclNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(IfStmtNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(ReturnStmtNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(ClassNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(FieldDeclNode<NodeInfo>& node) {}
// void TypeCheckerVisitor::visit(MethodDeclNode<NodeInfo>& node) {}
void TypeCheckerVisitor::visit(ConstructorDeclNode<NodeInfo>& node) {}
// void TypeCheckerVisitor::visit(ExprStmtNode<NodeInfo>& node) {}
