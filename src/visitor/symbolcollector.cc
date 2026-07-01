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
}

void SymbolCollector::collectProgram(ProgramNode& node) {
  ctx.push_scope();
  node.semantic.scope = ctx.get_current_scope();

  for (auto& stmt : node.children) collectStatement(*stmt);
  ctx.pop_scope();
}

void SymbolCollector::collectBlock(BlockNode& node) {
  ctx.push_scope();
  node.semantic.scope = ctx.get_current_scope();

  for (auto& stmt : node.statements) collectStatement(*stmt);
  ctx.pop_scope();
}

// void SymbolCollector::collectMethodDecl(MethodDeclNode& node) {
//   if (!node.declared_type) {
//     report_error("Missing type in method declaration", node.location);
//     return;
//   }

//   auto func = std::make_unique<FunctionSymbol>();
//   func->name = node.identifier.getValue();
//   func->location = node.location;
//   func->type = node.declared_type;
//   // func-> type = ;

//   for (const auto& param : node.param_list) {
//     func->param_types.push_back(param->declared_type);
//   }

//   Symbol* existing = ctx.lookup(node.identifier.getValue(), false);

//   if (!existing) {
//     FunctionSymbol* symbol = ctx.declare(node.identifier.getValue(),
//     func->type,
//                                          func->param_types, node.location);
//     if (!symbol) {
//       report_error("Cannot declare method " + node.identifier.getValue(),
//                    node.location);
//       return;
//     }

//     auto* func_sym = static_cast<FunctionSymbol*>(symbol);
//     func_sym->type = func->type;
//     func_sym->param_types = std::move(func->param_types);
//     func_sym->overloads.push_back(std::move(func));
//     node.semantic.data.variable.symbol = func_sym;
//   } else if (auto* func_sym = static_cast<FunctionSymbol*>(existing)) {
//     func_sym->overloads.push_back(std::move(func));
//   }

//   node.semantic.data.variable.symbol =
//       ctx.lookup(node.identifier.getValue(), false);

//   ctx.push_scope();
//   node.semantic.scope = ctx.get_current_scope();

//   for (auto& param : node.param_list) {
//     collectParamNode(*param);
//   }

//   if (node.body) collectBlock(*node.body);

//   ctx.pop_scope();
// }

void SymbolCollector::collectMethodDecl(MethodDeclNode& node) {
  if (!node.declared_type) {
    report_error("Missing return type in method declaration", node.location);
    return;
  }

  ctx.push_scope();

  std::vector<const Type*> param_types;

  for (auto& param : node.param_list) {
    if (!param->declared_type) {
      report_error("Missing type in parameter", param->location);
      continue;
    }

    Symbol* param_sym = ctx.declare(param->identifier.getValue(),
                                    param->declared_type, param->location);

    if (param_sym) {
      param->semantic.data.variable.symbol = param_sym;
    }

    param_types.push_back(param->declared_type);
  }

  FunctionSymbol* func_sym =
      ctx.declare(node.identifier.getValue(), node.declared_type, param_types,
                  node.location);

  if (func_sym) {
    node.semantic.data.variable.symbol = func_sym;
    std::string error;
    if (!ctx.method_table.add_method(func_sym, &error))
      report_error(error, node.location);
  } else {
    report_error("Failed to declare method " + node.identifier.getValue(),
                 node.location);
  }

  if (node.body) {
    collectBlock(*node.body);
  }

  ctx.pop_scope();
}

void SymbolCollector::collectParamNode(ParamNode& node) {
  if (!node.declared_type) {
    report_error("Missing type in parameter declaration", node.location);
    return;
  }

  auto symbol = ctx.declare(node.identifier.getValue(), node.declared_type,
                            node.location);

  if (!symbol) {
    report_error(
        "Redeclaration of parameter '" + node.identifier.getValue() + "'",
        node.location);
    return;
  }

  node.semantic.data.variable.symbol = symbol;
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
  if (node.expr) collectExpression(*node.expr);

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

  if (ctx.lookup(node.identifier.getValue(), true)) {
    report_error(
        "Redeclaration of variable '" + node.identifier.getValue() + "'",
        node.location);
    return;
  }

  auto symbol = ctx.declare(node.identifier.getValue(), node.declared_type,
                            node.location);
  node.semantic.data.variable.symbol = symbol;
}
