#include "visitor/symbolcollector.hh"

void SymbolCollector::collectStatement(StmtNode& stmt) {
  if (auto* node = dynamic_cast<BlockNode*>(&stmt))
    collectBlock(*node);
  else if (auto* node = dynamic_cast<IfStmtNode*>(&stmt))
    collectIfStmt(*node);
  else if (auto* node = dynamic_cast<WhileStmtNode*>(&stmt))
    collectWhileStmt(*node);
  else if (auto* node = dynamic_cast<ExprStmtNode*>(&stmt))
    collectExpression(*node->expr);
  else if (auto* node = dynamic_cast<MethodDeclNode*>(&stmt))
    collectMethodDecl(*node);
  else
    report_error("Unknown statment type", stmt.location);
}

void SymbolCollector::collectProgram(ProgramNode& node) {
  ctx.push_scope();
  for (auto& stmt : node.children) collectStatement(*stmt);
  ctx.pop_scope();
}

void SymbolCollector::collectBlock(BlockNode& node) {
  ctx.push_scope();
  for (auto& stmt : node.statements) collectStatement(*stmt);
  ctx.pop_scope();
}

void SymbolCollector::collectMethodDecl(MethodDeclNode& node) {
  if (!node.declared_type) {
    report_error("Missing type in method declaration", node.location);
    return;
  }

  // replace this with method key stuff
  auto symbol = ctx.declare(node.identifier.getValue(), node.declared_type,
                            node.location);
  if (!symbol) {
    report_error("Redeclaration of method '" + node.identifier.getValue() + "'",
                 node.location);
    return;
  }

  node.semantic.symbol = symbol;
}

void SymbolCollector::collectIfStmt(IfStmtNode& node) {
  collectExpression(*node.condition);
  collectStatement(*node.statement);

  if (node.else_stmt) collectStatement(*node.else_stmt);
}

void SymbolCollector::collectWhileStmt(WhileStmtNode& node) {
  collectExpression(*node.condition);
  if (node.statement) collectStatement(*node.statement);
}

void SymbolCollector::collectExpression(ExprNode& node) {
  if (auto* n = dynamic_cast<BinaryExprNode*>(&node))
    return collectBinaryExpr(*n);
  if (auto* n = dynamic_cast<AssignmentExprNode*>(&node))
    return collectAssignmentExpr(*n);
  if (auto* n = dynamic_cast<MethodCallNode*>(&node))
    return collectMethodCall(*n);
  if (auto* n = dynamic_cast<ArgumentNode*>(&node)) return collectArgument(*n);
  if (auto* n = dynamic_cast<VarDeclNode*>(&node)) return collectVarDecl(*n);
}

void SymbolCollector::collectBinaryExpr(BinaryExprNode& node) {
  collectExpression(*node.left);

  collectExpression(*node.right);
}

void SymbolCollector::collectAssignmentExpr(AssignmentExprNode& node) {
  collectExpression(*node.left);  // honestly i have no idea what this will do,
                                  // probably nothing good

  collectExpression(*node.right);
}

void SymbolCollector::collectMethodCall(MethodCallNode& node) {
  collectExpression(*node.expr);  // uhh no idea?

  for (auto& arg : node.arg_list) collectExpression(*arg.get());
}

void SymbolCollector::collectArgument(ArgumentNode& node) {
  collectExpression(*node.expr);
}

void SymbolCollector::collectVarDecl(VarDeclNode& node) {
  if (!node.declared_type) {
    report_error("Missing type in variable declaration", node.location);
    return;
  }

  auto symbol = ctx.declare(node.identifier.getValue(), node.declared_type,
                            node.location);
  if (!symbol) {
    report_error(
        "Redeclaration of variable '" + node.identifier.getValue() + "'",
        node.location);
    return;
  }

  node.semantic.symbol = symbol;
}
