#include "visitor/nameresolver.hh"

void NameResolver::resolveStatement(StmtNode& stmt) {
  if (auto* node = dynamic_cast<BlockNode*>(&stmt))
    resolveBlock(*node);
  else if (auto* node = dynamic_cast<VarDeclNode*>(&stmt))
    resolveVarDecl(*node);
  else if (auto* node = dynamic_cast<IfStmtNode*>(&stmt))
    resolveIfStmt(*node);
  else if (auto* node = dynamic_cast<WhileStmtNode*>(&stmt))
    resolveWhileStmt(*node);
  else if (auto* node = dynamic_cast<ReturnStmtNode*>(&stmt))
    resolveReturn(*node);
  else if (auto* node = dynamic_cast<ExprStmtNode*>(&stmt))
    resolveExprStmt(*node);
  else if (auto* node = dynamic_cast<MethodDeclNode*>(&stmt))
    resolveMethodDecl(*node);
  else
    report_error("Unknown statment type", stmt.location);
}

const Type* NameResolver::resolveExpression(ExprNode& expr) {
  if (auto* node = dynamic_cast<BinaryExprNode*>(&expr))
    return resolveBinaryExpr(*node);
  if (auto* node = dynamic_cast<UnaryExprNode*>(&expr))
    return resolveUnaryExpr(*node);
  if (auto* node = dynamic_cast<LiteralExprNode*>(&expr))
    return resolveLiteralExpr(*node);
  if (auto* node = dynamic_cast<IdentifierExprNode*>(&expr))
    return resolveIdentifierExpr(*node);
  if (auto* node = dynamic_cast<AssignmentExprNode*>(&expr))
    return resolveAssignmentExpr(*node);
  if (auto* node = dynamic_cast<MethodCallNode*>(&expr))
    return resolveMethodCall(*node);
  if (auto* node = dynamic_cast<ArgumentNode*>(&expr))
    return resolveArgument(*node);

  report_error("Unknown expression type", expr.location);
  return ctx.get_void_type();
}

void NameResolver::resolveProgram(ProgramNode& node) {
  for (auto& stmt : node.children) resolveStatement(*stmt);
}

void NameResolver::resolveBlock(BlockNode& node) {
  for (auto& stmt : node.statements) resolveStatement(*stmt);
}
