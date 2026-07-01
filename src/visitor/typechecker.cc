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
  if (auto* node = dynamic_cast<VarDeclNode*>(&expr))
    return checkVarDecl(*node);

  report_error("Unknown expression type", expr.location);
  return ctx.get_void_type();
}

void TypeChecker::checkProgram(ProgramNode& node) {
  ctx.set_current_scope(node.semantic.scope);
  for (auto& child : node.children) checkStatement(*child);
  ctx.set_current_scope(node.semantic.scope->get_parent());
}

void TypeChecker::checkBlock(BlockNode& node) {
  ctx.set_current_scope(node.semantic.scope);
  for (auto& stmt : node.statements) checkStatement(*stmt);
  ctx.set_current_scope(node.semantic.scope->get_parent());
}

const Type* TypeChecker::checkVarDecl(VarDeclNode& node) {
  if (node.initializer) checkExpression(*node.initializer);

  if (!node.declared_type->is_compatible_with(
          *node.initializer->semantic.declared_type)) {
    report_error("Type '" + node.declared_type->to_string() +
                     "' does not match initializer type '" +
                     node.initializer->semantic.declared_type->to_string() +
                     "' in '" + node.identifier.getValue() + "' declaration",
                 node.location);
    return ctx.get_void_type();
  }

  // for now we'll only allow exact type matches, in the future we do need stuff
  // like MyClass a = new MyClass2(); where MyClass2 : MyClass
  node.semantic.declared_type = node.declared_type;
  return node.semantic.declared_type;
}

void TypeChecker::checkIfStmt(IfStmtNode& node) {
  if (node.condition) checkExpression(*node.condition);
  if (!node.condition->semantic.declared_type->is_compatible_with(
          *ctx.get_bool_type())) {
    report_error(
        "If statement condition is not compatible with boolean, condition "
        "type: '" +
            node.condition->semantic.declared_type->to_string() + "'",
        node.condition->location);
    return;
  }

  if (node.statement) checkStatement(*node.statement);
  if (node.else_stmt) checkStatement(*node.else_stmt);
}

void TypeChecker::checkWhileStmt(WhileStmtNode& node) {
  if (node.condition) checkExpression(*node.condition);
  if (!node.condition->semantic.declared_type->is_compatible_with(
          *ctx.get_bool_type())) {
    report_error(
        "While statement condition is not compatible with boolean, condition "
        "type: '" +
            node.condition->semantic.declared_type->to_string() + "'",
        node.condition->location);
    return;
  }
}

void TypeChecker::checkReturn(ReturnStmtNode& node) {
  if (!current_function) {
    report_error("Tried to return whilst not in function!", node.location);
    return;
  }

  if (!node.ret) {
    report_error("No expression to return", node.ret->location);
    return;
  }

  checkExpression(*node.ret);

  if (!node.ret->semantic.declared_type) {
    report_error("Return type not found for return statement", node.location);
    return;
  }

  if (!node.ret->semantic.declared_type->is_compatible_with(
          *current_function->type)) {
    report_error("Invalid return type '" +
                     node.ret->semantic.declared_type->to_string() +
                     "' for function '" + current_function->name +
                     "' with type '" + current_function->type->to_string() +
                     "'",
                 node.location);
    return;
  }

  node.ret->semantic.declared_type = current_function->type;
}

void TypeChecker::checkExprStmt(ExprStmtNode& node) {
  if (node.expr) checkExpression(*node.expr);
  node.semantic.declared_type = node.expr->semantic.declared_type;
}

void TypeChecker::checkMethodDecl(MethodDeclNode& node) {
  if (!node.semantic.data.variable.symbol) {
    report_error("Function '" + node.identifier.getValue() + "' not resolved",
                 node.location);
    return;
  }

  current_function =
      static_cast<FunctionSymbol*>(node.semantic.data.variable.symbol);

  if (node.body) checkStatement(*node.body);
}

const Type* TypeChecker::checkBinaryExpr(BinaryExprNode& node) {
  const Type* left = nullptr;
  const Type* right = nullptr;
  if (node.left) left = checkExpression(*node.left);
  if (node.right) right = checkExpression(*node.right);

  if (!left || !right) {
    report_error("Binary expr side does not resolve to a type", node.location);
    return ctx.get_void_type();
  }

  // not sure if it makes a difference, but the right should be compatible with
  // left, maybe for stuff like "left" + 2 == "left2"
  const Type* result = check_binary_op(node.op.getType(), *left, *right);
  if (result->equals(*ctx.get_void_type())) {
    report_error("Right type '" + right->to_string() +
                     "' is not compatible with left type '" +
                     left->to_string() + "' with operator '" +
                     node.op.getValue() + "'",
                 node.location);
    return ctx.get_void_type();
  }

  node.semantic.declared_type = result;
  return node.semantic.declared_type;
}

const Type* TypeChecker::checkUnaryExpr(UnaryExprNode& node) {
  const Type* operand = nullptr;
  if (node.operand) operand = checkExpression(*node.operand);

  if (!operand || operand->equals(*ctx.get_void_type()) ||
      !operand->equals(*ctx.get_int32_type())) {
    report_error("Unary expression operand type is not int",
                 node.operand->location);
    return ctx.get_void_type();
  }

  node.semantic.declared_type = operand;
  return node.semantic.declared_type;
}

const Type* TypeChecker::checkLiteralExpr(LiteralExprNode& node) {
  node.semantic.declared_type = node.result_type;
  return node.semantic.declared_type;
}

const Type* TypeChecker::checkIdentifierExpr(IdentifierExprNode& node) {
  if (!node.semantic.data.variable.symbol) {
    report_error(
        "Symbol not found for identifier '" + node.identifier.getValue() + "'",
        node.location);
    return ctx.get_void_type();
  }
  node.semantic.declared_type = node.semantic.data.variable.symbol->type;
  return node.semantic.declared_type;
}

const Type* TypeChecker::checkAssignmentExpr(AssignmentExprNode& node) {
  if (!node.semantic.data.variable.symbol) {
    report_error("Symbol not found for assignment", node.location);
    return ctx.get_void_type();
  }

  if (node.left) checkExpression(*node.left);
  if (node.right) checkExpression(*node.right);

  if (!node.left->semantic.declared_type->is_compatible_with(
          *node.right->semantic.declared_type)) {
    report_error("Left type '" +
                     node.left->semantic.declared_type->to_string() +
                     "' is not compatible with right type '" +
                     node.right->semantic.declared_type->to_string() + "'",
                 node.location);
    return ctx.get_void_type();
  }

  node.semantic.declared_type = node.left->semantic.declared_type;
  return node.semantic.declared_type;
}

const Type* TypeChecker::checkMethodCall(MethodCallNode& node) {
  std::vector<const Type*> arg_types;
  for (auto& arg : node.arg_list) {
    arg_types.push_back(checkExpression(*arg));
  }

  // for now this will do, update when classes are done
  std::string owner = "global";

  const FunctionSymbol* candidate =
      ctx.method_table.find_overload("", node.identifier.getValue(), arg_types);

  if (!candidate) {
    report_error("Cannot find candidate for method call with identifier '" +
                     node.identifier.getValue() + "'",
                 node.location);
    return ctx.get_void_type();
  }

  node.semantic.data.call.callee = const_cast<FunctionSymbol*>(candidate);
  node.semantic.declared_type = candidate->type;
  return node.semantic.declared_type;
}

const Type* TypeChecker::checkArgument(ArgumentNode& node) {
  const Type* result = nullptr;
  if (node.expr) result = checkExpression(*node.expr);

  if (!result) {
    report_error("Type not found for argument node", node.location);
    return ctx.get_void_type();
  }

  node.semantic.declared_type = result;
  return node.semantic.declared_type;
}
