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

void NameResolver::resolveExpression(ExprNode& expr) {
  if (auto* node = dynamic_cast<BinaryExprNode*>(&expr))
    resolveBinaryExpr(*node);
  if (auto* node = dynamic_cast<UnaryExprNode*>(&expr)) resolveUnaryExpr(*node);
  // if (auto* node = dynamic_cast<LiteralExprNode*>(&expr))
  //   resolveLiteralExpr(*node);
  if (auto* node = dynamic_cast<IdentifierExprNode*>(&expr))
    resolveIdentifierExpr(*node);
  if (auto* node = dynamic_cast<AssignmentExprNode*>(&expr))
    resolveAssignmentExpr(*node);
  if (auto* node = dynamic_cast<MethodCallNode*>(&expr))
    resolveMethodCall(*node);
  // if (auto* node = dynamic_cast<ArgumentNode*>(&expr))
  // resolveArgument(*node);
}

void NameResolver::resolveProgram(ProgramNode& node) {
  ctx.set_current_scope(node.semantic.scope);
  for (auto& stmt : node.children) resolveStatement(*stmt);
  ctx.set_current_scope(node.semantic.scope->get_parent());
}

void NameResolver::resolveBlock(BlockNode& node) {
  ctx.set_current_scope(node.semantic.scope);
  for (auto& stmt : node.statements) resolveStatement(*stmt);
  ctx.set_current_scope(node.semantic.scope->get_parent());
}

void NameResolver::resolveVarDecl(VarDeclNode& node) {
  if (node.initializer) resolveExpression(*node.initializer);
}

void NameResolver::resolveIfStmt(IfStmtNode& node) {
  if (node.condition) resolveExpression(*node.condition);
  if (node.statement) resolveStatement(*node.statement);
  if (node.else_stmt) resolveStatement(*node.else_stmt);
}

void NameResolver::resolveWhileStmt(WhileStmtNode& node) {
  if (node.condition) resolveExpression(*node.condition);
  if (node.statement) resolveStatement(*node.statement);
}

void NameResolver::resolveReturn(ReturnStmtNode& node) {
  if (node.ret) resolveExpression(*node.ret);
}

void NameResolver::resolveExprStmt(ExprStmtNode& node) {
  if (node.expr) resolveExpression(*node.expr);
}

void NameResolver::resolveMethodDecl(MethodDeclNode& node) {
  if (node.body) resolveStatement(*node.body);
}

void NameResolver::resolveBinaryExpr(BinaryExprNode& node) {
  if (node.left) resolveExpression(*node.left);
  if (node.right) resolveExpression(*node.right);
}

void NameResolver::resolveUnaryExpr(UnaryExprNode& node) {
  if (node.operand) resolveExpression(*node.operand);
}

void NameResolver::resolveIdentifierExpr(IdentifierExprNode& node) {
  Symbol* sym = ctx.lookup(node.identifier.getValue());

  if (!sym) {
    report_error(
        "Variable '" + node.identifier.getValue() + "' not found in scope",
        node.location);
    return;
  }

  node.semantic.data.variable.symbol = sym;
}

void NameResolver::resolveAssignmentExpr(AssignmentExprNode& node) {
  if (node.left) resolveExpression(*node.left);
  if (node.right) resolveExpression(*node.right);
}

void NameResolver::resolveMethodCall(MethodCallNode& node) {
  if (node.expr) resolveExpression(*node.expr);

  for (auto& arg : node.arg_list) resolveExpression(*arg->expr);
}
