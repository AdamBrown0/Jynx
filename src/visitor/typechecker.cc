#include "visitor/typechecker.hh"

void TypeChecker::checkStatement(StmtNode& stmt) {
  if (auto* node = dynamic_cast<BlockNode*>(&stmt))
    checkBlock(*node);
  else if (auto* node = dynamic_cast<VarDeclNode*>(&stmt))
    checkVarDecl(*node);
  else if (auto* node = dynamic_cast<IfStmtNode*>(&stmt))
    checkIfStmt(*node);
  else if (auto* node = dynamic_cast<WhileStmtNode*>(&stmt))
    checkWhileStmt(*node);
  else if (auto* node = dynamic_cast<ReturnStmtNode*>(&stmt))
    checkReturn(*node);
  else if (auto* node = dynamic_cast<ExprStmtNode*>(&stmt))
    checkExprStmt(*node);
  else if (auto* node = dynamic_cast<MethodDeclNode*>(&stmt))
    checkMethodDecl(*node);
  else
    report_error("Unknown statment type", stmt.location);
}

const Type* TypeChecker::checkExpression(ExprNode& expr) {
  if (auto* node = dynamic_cast<BinaryExprNode*>(&expr))
    return checkBinaryExpr(*node);
  if (auto* node = dynamic_cast<UnaryExprNode*>(&expr))
    return checkUnaryExpr(*node);
  if (auto* node = dynamic_cast<LiteralExprNode*>(&expr))
    return checkLiteralExpr(*node);
  if (auto* node = dynamic_cast<IdentifierExprNode*>(&expr))
    return checkIdentifierExpr(*node);
  if (auto* node = dynamic_cast<AssignmentExprNode*>(&expr))
    return checkAssignmentExpr(*node);
  if (auto* node = dynamic_cast<MethodCallNode*>(&expr))
    return checkMethodCall(*node);
  if (auto* node = dynamic_cast<ArgumentNode*>(&expr))
    return checkArgument(*node);

  report_error("Unknown expression type", expr.location);
  return ctx.get_void_type();
}
