#include "visitor/typechecker.hh"

void TypeChecker::checkStatement(StmtNode<NodeInfo>& stmt) {
  if (auto* node = dynamic_cast<BlockNode<NodeInfo>*>(&stmt))
    checkBlock(*node);
  else if (auto* node = dynamic_cast<VarDeclNode<NodeInfo>*>(&stmt))
    checkVarDecl(*node);
  else if (auto* node = dynamic_cast<IfStmtNode<NodeInfo>*>(&stmt))
    checkIfStmt(*node);
  else if (auto* node = dynamic_cast<WhileStmtNode<NodeInfo>*>(&stmt))
    checkWhileStmt(*node);
  else if (auto* node = dynamic_cast<ReturnStmtNode<NodeInfo>*>(&stmt))
    checkReturn(*node);
  else if (auto* node = dynamic_cast<ExprStmtNode<NodeInfo>*>(&stmt))
    checkExprStmt(*node);
  else if (auto* node = dynamic_cast<MethodDeclNode<NodeInfo>*>(&stmt))
    checkMethodDecl(*node);
  else
    report_error("Unknown statment type", stmt.location);
}

const Type* TypeChecker::checkExpression(ExprNode<NodeInfo>& expr) {
  if (auto* node = dynamic_cast<BinaryExprNode<NodeInfo>*>(&expr))
    return checkBinaryExpr(*node);
  if (auto* node = dynamic_cast<UnaryExprNode<NodeInfo>*>(&expr))
    return checkUnaryExpr(*node);
  if (auto* node = dynamic_cast<LiteralExprNode<NodeInfo>*>(&expr))
    return checkLiteralExpr(*node);
  if (auto* node = dynamic_cast<IdentifierExprNode<NodeInfo>*>(&expr))
    return checkIdentifierExpr(*node);
  if (auto* node = dynamic_cast<AssignmentExprNode<NodeInfo>*>(&expr))
    return checkAssignmentExpr(*node);
  if (auto* node = dynamic_cast<MethodCallNode<NodeInfo>*>(&expr))
    return checkMethodCall(*node);
  if (auto* node = dynamic_cast<ArgumentNode<NodeInfo>*>(&expr))
    return checkArgument(*node);

  report_error("Unknown expression type", expr.location);
  return ctx.get_void_type();
}
