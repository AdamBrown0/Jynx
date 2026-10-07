#include "ir/gen.hh"

#include "diagnostics.hh"

std::unique_ptr<ir::Module> IRGenerator::generateIR(ProgramNode& program) {
  auto mod = std::make_unique<ir::Module>();
  current_module = mod.get();

  generateProgram(program);

  current_block = nullptr;
  current_function = nullptr;
  current_module = nullptr;

  return mod;
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
    generateReturnStmt(*node);
    return nullptr;
  }

  if (auto* node = dynamic_cast<IfStmtNode*>(&stmt)) {
    generateIfStmt(*node);
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

void IRGenerator::generateReturnStmt(ReturnStmtNode& node) {
  ir::Value* rhs = nullptr;
  if (node.ret) rhs = generateExpression(*node.ret);

  auto ret = std::make_unique<ir::Instruction>(
      ir::Opcode::Ret, current_function->returnType, current_block);

  if (rhs) ret->addOperand(rhs);

  current_block->appendInstruction(std::move(ret));
}

void IRGenerator::generateIfStmt(IfStmtNode& node) {
  auto* if_block = createBlock("if.then");
  auto* else_block = createBlock("if.else");
  auto* end_block = createBlock("if.end");

  auto* condition = generateExpression(*node.condition);

  auto condbr = std::make_unique<ir::Instruction>(
      ir::Opcode::CondBr, ctx.get_void_type(), current_block);
  condbr->addOperand(condition);
  condbr->addOperand(if_block);
  condbr->addOperand(else_block);

  current_block->appendInstruction(std::move(condbr));

  ir::BasicBlock::addEdge(current_block, if_block);
  ir::BasicBlock::addEdge(current_block, else_block);

  current_block = if_block;
  generateStatement(*node.statement);

  auto merge = std::make_unique<ir::Instruction>(
      ir::Opcode::Br, ctx.get_void_type(), current_block);
  merge->addOperand(end_block);
  current_block->appendInstruction(std::move(merge));

  ir::BasicBlock::addEdge(current_block, end_block);

  current_block = else_block;
  if (node.else_stmt) generateStatement(*node.else_stmt);

  merge = std::make_unique<ir::Instruction>(ir::Opcode::Br, ctx.get_void_type(),
                                            current_block);
  merge->addOperand(end_block);
  current_block->appendInstruction(std::move(merge));

  ir::BasicBlock::addEdge(current_block, end_block);

  current_block = end_block;
}

ir::Value* IRGenerator::generateExpression(ExprNode& expr) {
  if (auto* node = dynamic_cast<BinaryExprNode*>(&expr))
    return generateBinaryExpr(*node);

  if (auto* node = dynamic_cast<UnaryExprNode*>(&expr))
    return generateUnaryExpr(*node);

  if (auto* node = dynamic_cast<LiteralExprNode*>(&expr))
    return generateLiteralExpr(*node);

  if (auto* node = dynamic_cast<AssignmentExprNode*>(&expr))
    return generateAssignmentExpr(*node);

  if (auto* node = dynamic_cast<VarDeclNode*>(&expr))
    return generateVarDecl(*node);

  if (auto* node = dynamic_cast<IdentifierExprNode*>(&expr))
    return generateIdentifierExpr(*node);

  // if (auto* node = dynamic_cast<MethodCallNode*>(&expr))
  //   return generateMethodCall(*node);

  Diagnostics::instance().report_error(LOG_KIND, "Unknown expression type",
                                       expr.location);
  return nullptr;
}

ir::Value* IRGenerator::generateExprStmt(ExprStmtNode& node) {
  if (!node.expr) return nullptr;
  return generateExpression(*node.expr);
}

ir::Value* IRGenerator::generateVarDecl(VarDeclNode& node) {
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

ir::Value* IRGenerator::generateIdentifierExpr(IdentifierExprNode& node) {
  auto it = locals.find(node.semantic.data.variable.symbol);

  if (it == locals.end()) {
    Diagnostics::instance().report_error(
        LOG_KIND, "Identifier found for non-existent variable", node.location);
    return nullptr;
  }

  ir::Value* address = it->second;

  auto* load = emit(ir::Opcode::Load, node.semantic.declared_type, nextTemp());

  load->addOperand(address);

  return load;
}

ir::Value* IRGenerator::generateAssignmentExpr(AssignmentExprNode& node) {
  auto* identifier = dynamic_cast<IdentifierExprNode*>(node.left.get());

  if (!identifier) {
    Diagnostics::instance().report_error(
        LOG_KIND, "Tried to assign to non-identifier", node.location);
    return nullptr;
  }

  auto it = locals.find(identifier->semantic.data.variable.symbol);

  if (it == locals.end()) {
    Diagnostics::instance().report_error(
        LOG_KIND, "Tried to assign to non-existent identifier", node.location);
    return nullptr;
  }

  auto* rhs = generateExpression(*node.right);

  auto store = std::make_unique<ir::Instruction>(
      ir::Opcode::Store, ctx.get_void_type(), current_block);

  store->addOperand(it->second);
  store->addOperand(rhs);

  current_block->appendInstruction(std::move(store));

  return rhs;
}

// ir::Value* IRGenerator::generateMethodCall(MethodCallNode& node){

// }

ir::Value* IRGenerator::generateLiteralExpr(LiteralExprNode& node) {
  return makeConstant(node.semantic.declared_type,
                      node.literal_token.getValue());
}

ir::Value* IRGenerator::generateBinaryExpr(BinaryExprNode& node) {
  ir::Value* left = generateExpression(*node.left);

  ir::Value* right = generateExpression(*node.right);

  ir::Opcode opcode;

  switch (node.op.getType()) {
    case TokenType::TOKEN_PLUS:
      opcode = ir::Opcode::Add;
      break;
    case TokenType::TOKEN_MINUS:
      opcode = ir::Opcode::Sub;
      break;
    case TokenType::TOKEN_MULTIPLY:
      opcode = ir::Opcode::Mul;
      break;
    case TokenType::TOKEN_DIVIDE:
      opcode = ir::Opcode::Div;
      break;
    default:
      Diagnostics::instance().report_error(
          LOG_KIND,
          "Unsupported binary operator '" + std::string(node.op.to_string()) +
              "'",
          node.location);
      return nullptr;
  }

  auto* result = emit(opcode, node.semantic.declared_type, nextTemp());

  result->addOperand(left);
  result->addOperand(right);

  return result;
}

ir::Value* IRGenerator::generateUnaryExpr(UnaryExprNode& node) {
  auto* operand = generateExpression(*node.operand);

  if (node.op.getType() != TokenType::TOKEN_MINUS) {
    Diagnostics::instance().report_error(LOG_KIND, "Unsupported unary operator",
                                         node.location);
    return nullptr;
  }

  auto* zero = makeConstant(node.semantic.declared_type, "0");

  auto* result = emit(ir::Opcode::Sub, node.semantic.declared_type, nextTemp());

  result->addOperand(zero);
  result->addOperand(operand);

  return result;
}

ir::BasicBlock* IRGenerator::createBlock(const std::string& prefix) {
  return current_function->createBlock(prefix + "." +
                                       std::to_string(block_counter++));
}

ir::Value* IRGenerator::makeConstant(const Type* type,
                                     const std::string& value) {
  // TODO: change this for different Constant<type> instructions
  return CompilerContext::instance().getOrCreateConstant(type, value);
}

ir::Instruction* IRGenerator::emit(ir::Opcode opcode, const Type* type,
                                   std::string name) {
  auto instruction = std::make_unique<ir::Instruction>(
      opcode, type, current_block, std::move(name));

  auto* raw = instruction.get();
  current_block->appendInstruction(std::move(instruction));
  return raw;
}

std::string IRGenerator::nextTemp() {
  return "t" + std::to_string(temp_counter++);
}
