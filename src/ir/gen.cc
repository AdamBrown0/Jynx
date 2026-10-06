#include "ir/gen.hh"

#include "diagnostics.hh"

ir::Module* IRGenerator::generateIR(ProgramNode& program) {
  current_module = new ir::Module();

  generateProgram(program);

  current_block = nullptr;
  current_function = nullptr;

  return current_module;
}

void IRGenerator::generateProgram(ProgramNode& node) {
  // allows for initial program setup
  auto* main =
      current_module->create_function("jynx_main", ctx.get_int32_type());

  current_function = main;
  current_block = main->createBlock("entry");

  for (auto& child : node.children) {
    generateStatement(*child);
  }

  // TODO: this should return the value from the real main function
  if (current_block &&
      (current_block->instructions.empty() ||
       current_block->instructions.back()->op != ir::Opcode::Ret)) {
    auto ret = std::make_unique<ir::Instruction>(
        ir::Opcode::Ret, ctx.get_int32_type(), current_block);

    ret->addOperand(makeConstant(ctx.get_int32_type(), "0"));
    current_block->appendInstruction(std::move(ret));
  }

  current_block = nullptr;
  current_function = nullptr;
}

ir::Value* IRGenerator::generateStatement(StmtNode& stmt) {
  if (auto* node = dynamic_cast<BlockNode*>(&stmt)) {
    generateBlock(*node);
    return nullptr;
  }

  if (auto* node = dynamic_cast<MethodDeclNode*>(&stmt)) {
    generateMethodDecl(*node);
    return nullptr;
  }

  if (auto* node = dynamic_cast<ReturnStmtNode*>(&stmt)) {
    // generateReturnStmt(*node);
    return nullptr;
  }

  if (auto* node = dynamic_cast<IfStmtNode*>(&stmt)) {
    // generateIfStmt(*node);
    return nullptr;
  }

  if (auto* node = dynamic_cast<WhileStmtNode*>(&stmt)) {
    // generateWhileStmt(*node);
    return nullptr;
  }

  if (auto* node = dynamic_cast<ExprStmtNode*>(&stmt)) {
    return generateExprStmt(*node);
  }

  if (auto* node = dynamic_cast<VarDeclNode*>(&stmt)) {
    return generateVarDecl(*node);
  }

  Diagnostics::instance().report_error(LOG_KIND, "Unknown statement type",
                                       stmt.location);

  return nullptr;
}

void IRGenerator::generateBlock(BlockNode& node) {
  // dont create the label, just emit instructions
  for (auto& stmt : node.statements) generateStatement(*stmt);
}

void IRGenerator::generateMethodDecl(MethodDeclNode& node) {
  if (!node.semantic.data.variable.symbol) return;

  auto* symbol =
      static_cast<FunctionSymbol*>(node.semantic.data.variable.symbol);

  auto* function = current_module->create_function(symbol->name, symbol->type);

  auto* previous_function = current_function;
  auto* previous_block = current_block;

  current_function = function;
  current_block = function->createBlock("entry");

  locals.clear();

  if (node.body) generateBlock(*node.body);

  if (current_block->instructions.empty() ||
      current_block->instructions.back()->op != ir::Opcode::Ret) {
    auto ret = std::make_unique<ir::Instruction>(
        ir::Opcode::Ret, ctx.get_void_type(), current_block);
    current_block->appendInstruction(std::move(ret));
  }

  current_function = previous_function;
  current_block = previous_block;
}

ir::Value* IRGenerator::generateExpression(ExprNode& expr) {
  LOG_DEBUG("WE IN GEN EXPR");
  // if (auto* node = dynamic_cast<BinaryExprNode*>(&expr))
  // return generateBinaryExpr(*node);

  // if (auto* node = dynamic_cast<UnaryExprNode*>(&expr))
  // return generateUnaryExpr(*node);

  if (auto* node = dynamic_cast<LiteralExprNode*>(&expr))
    return generateLiteralExpr(*node);

  // if (auto* node = dynamic_cast<AssignmentExprNode*>(&expr))
  // return generateAssignmentExpr(*node);

  if (auto* node = dynamic_cast<VarDeclNode*>(&expr))
    return generateVarDecl(*node);

  // if (auto* node = dynamic_cast<IdentifierExprNode*>(&expr))
  // return generateIdentifierExpr(*node);

  // if (auto* node = dynamic_cast<MethodCallNode*>(&expr))
  // return generateMethodCall(*node);

  Diagnostics::instance().report_error(LOG_KIND, "Unknown expression type",
                                       expr.location);
  return nullptr;
}

ir::Value* IRGenerator::generateExprStmt(ExprStmtNode& node) {
  LOG_DEBUG("WE IN exprstmt!!");
  if (!node.expr) return nullptr;
  LOG_DEBUG("AFTER NODE.EXPR CHECK");
  return generateExpression(*node.expr);
}

ir::Value* IRGenerator::generateVarDecl(VarDeclNode& node) {
  LOG_DEBUG("WE IN vardecl!!");

  if (!node.declared_type) {
    Diagnostics::instance().report_error(
        LOG_KIND, "Variable declaration has no type", node.location);
  }

  const Symbol* symbol = node.semantic.data.variable.symbol;

  auto* address =
      emit(ir::Opcode::Alloca, ctx.make_pointer_type(node.declared_type),
           node.identifier.getValue());

  locals[symbol] = address;

  if (node.initializer) {
    auto* value = generateExpression(*node.initializer);

    auto store = std::make_unique<ir::Instruction>(
        ir::Opcode::Store, ctx.get_void_type(), current_block);

    store->addOperand(address);
    store->addOperand(value);

    current_block->appendInstruction(std::move(store));
  }

  return address;
}

ir::Value* IRGenerator::generateLiteralExpr(LiteralExprNode& node) {
  LOG_DEBUG("WE IN LITERAL!!");
  return makeConstant(node.semantic.declared_type,
                      node.literal_token.getValue());
}

ir::Value* IRGenerator::makeConstant(const Type* type,
                                     const std::string& value) {
  // TODO: change this for different Constant<type> instructions
  return new ir::Value(type, value);
}

ir::Instruction* IRGenerator::emit(ir::Opcode opcode, const Type* type,
                                   std::string name) {
  auto instruction = std::make_unique<ir::Instruction>(
      opcode, type, current_block, std::move(name));

  auto* raw = instruction.get();
  current_block->appendInstruction(std::move(instruction));
  return raw;
}
