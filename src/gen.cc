#include "gen.hh"

#include <sstream>

#include "ast.hh"
#include "log.hh"

std::string CodeGenerator::generate(const ProgramNode<NodeInfo> &root) {
  LOG_DEBUG("[GEN] Resetting areas");
  // reset
  text_section.str("");
  text_section.clear();
  rodata_section.str("");
  rodata_section.clear();
  current_stack_offset = 0;
  label_counter = 0;
  setupRegisters();

  LOG_DEBUG("[GEN] Boilerplate");
  text_section << ".intel_syntax noprefix\n";
  text_section << ".section .text\n";
  text_section << ".global _start\n";
  // Enter main function scope
  enter_scope();

  LOG_DEBUG("[GEN] Accepting nodes");
  auto &root_noconst = const_cast<ProgramNode<NodeInfo> &>(root);
  root_noconst.accept(*this);

  // Exit main function scope
  exit_scope();

  LOG_DEBUG("[GEN] Eval stack");
  // if (!eval_stack.empty()) {
  //   std::string top = eval_stack.back();
  //   eval_stack.pop_back();
  //   emitMove("rax", top);
  //   freeRegister(top);
  // } else {
  //   emitMove("rax", "0");
  // }

  LOG_DEBUG("[GEN] Leaving");
  // leave
  // emit("leave");
  // emit("ret\n");

  // generate entrypoint
  emitLabel("\n_start");
  emitCall("main");
  emitMove("rdi", "rax");
  emitMove("rax", "60");
  emit("syscall");
  emit("\n");

  std::ostringstream out;
  out << text_section.str();
  if (!literal_pool_emission.empty()) {
    out << ".section .rodata\n";
    for (auto &p : literal_pool_emission) {
      emitRodataLiteral(p.first, p.second);
    }
    out << rodata_section.str();
    out << "\n";
  }

  return out.str();
}

void CodeGenerator::visit(ProgramNode<NodeInfo> &node) {
  LOG_DEBUG("Code generator: Visiting ProgramNode with {} children",
            node.children.size());
  // nop
  // this is handled in the programnode struct
}

void CodeGenerator::visit(VarDeclNode<NodeInfo> &node) {
  std::string name = node.identifier.getValue();
  // If this is a string variable, allocate two slots and store descriptor.
  bool isStringDecl = (node.extra.resolved_type == TokenType::TOKEN_STRING) ||
                      (node.type_token.getValue() == "string");
  if (isStringDecl) {
    ensureStringVarSlots(name);
    if (node.initializer) {
      storeCurrentStringToVar(name);
      // Var decl does not leave a value on eval stack; pop any marker if
      // present.
      if (!eval_stack.empty() && eval_stack.back() == "$str") {
        eval_stack.pop_back();
      }
    } else {
      // zero initialize - need to get slots first
      auto [ptrOff, lenOff] = ensureStringVarSlots(name);
      emit("mov QWORD PTR " + formatSlot(ptrOff) + ", 0");
      emit("mov QWORD PTR " + formatSlot(lenOff) + ", 0");
    }
  } else {
    std::string loc = formatSlot(node);
    if (node.initializer) {
      std::string r = eval_stack.back();
      eval_stack.pop_back();
      emitMove(loc, r);
      freeRegister(r);
    } else {
      emitMove(loc, "0");
    }
  }
}

void CodeGenerator::visit(BinaryExprNode<NodeInfo> &node) {
  std::string right = eval_stack.back();
  eval_stack.pop_back();
  std::string left = eval_stack.back();
  eval_stack.pop_back();
  std::string dest = left;

  if (isComparisonOp(node.op.getType()))
    emitCompare(left, right);
  else {
    std::string dest = left;
    emitArithmetic(node.op.getType(), left, right, dest);
    eval_stack.push_back(left);
  }
  freeRegister(right);
}

void CodeGenerator::visit(LiteralExprNode<NodeInfo> &node) {
  if (node.literal_token.getType() == TokenType::TOKEN_INT) {
    std::string r = allocateRegister(true);
    if (r.empty()) r = "rax";
    emitMove(r, node.literal_token.getValue());
    eval_stack.push_back(r);
  } else if (node.literal_token.getType() == TokenType::TOKEN_STRING) {
    // Load string literal into RAX (ptr) and RDX (len)
    loadStringLiteral(node.literal_token.getValue());
    eval_stack.push_back("$str");
  } else {  // only support int for now
    LOG_WARN("[GEN] Unsupported type: {}", node.literal_token.to_string());
    std::string r = allocateRegister(true);
    if (r.empty()) r = "rax";
    emit("xor " + r + ", " + r);
    eval_stack.push_back(r);
  }
}

void CodeGenerator::visit(IdentifierExprNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited ident");

  std::string name = node.identifier.getValue();
  if (isStringVariable(name)) {
    loadStringFromVar(name);  // RAX/RDX
    eval_stack.push_back("$str");
  } else {
    std::string var_location = formatSlot(node);
    std::string r = allocateRegister(true);
    if (r.empty()) r = "rax";
    emitMove(r, var_location);
    eval_stack.push_back(r);
  }
}

void CodeGenerator::visit(IfStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited ifstmt");
  if (if_stack.empty()) return;
  const auto &ctx = if_stack.back();
  if (auto *cond =
          dynamic_cast<BinaryExprNode<NodeInfo> *>(node.condition.get())) {
    switch (cond->op.getType()) {
      case TokenType::TOKEN_DEQ:
        emitConditionalJump("ne", ctx.false_label);
        break;
      case TokenType::TOKEN_NEQ:
        emitConditionalJump("e", ctx.false_label);
        break;
      case TokenType::TOKEN_GEQ:
        emitConditionalJump("l", ctx.false_label);
        break;
      case TokenType::TOKEN_GT:
        emitConditionalJump("le", ctx.false_label);
        break;
      case TokenType::TOKEN_LEQ:
        emitConditionalJump("g", ctx.false_label);
        break;
      case TokenType::TOKEN_LT:
        emitConditionalJump("ge", ctx.false_label);
        break;
      default:
        LOG_ERROR("[GEN] Expected conditional operator, found {}",
                  cond->op.to_string());
    }
  }
}

void CodeGenerator::visit(WhileStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited whilestmt");
  if (while_stack.empty()) return;
  const auto &ctx = while_stack.back();
  if (auto *cond =
          dynamic_cast<BinaryExprNode<NodeInfo> *>(node.condition.get())) {
    switch (cond->op.getType()) {
      case TokenType::TOKEN_DEQ:
        emitConditionalJump("ne", ctx.end_label);
        break;
      case TokenType::TOKEN_GEQ:
        emitConditionalJump("l", ctx.end_label);
        break;
      case TokenType::TOKEN_GT:
        emitConditionalJump("le", ctx.end_label);
        break;
      case TokenType::TOKEN_LEQ:
        emitConditionalJump("g", ctx.end_label);
        break;
      case TokenType::TOKEN_LT:
        emitConditionalJump("ge", ctx.end_label);
        break;
      default:
        LOG_ERROR("[GEN] Expected conditional operator, found {}",
                  cond->op.to_string());
    }
  }
}

void CodeGenerator::visit(BlockNode<NodeInfo> &node) { (void)node; }

void CodeGenerator::visit(AssignmentExprNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited assignmentExpr");
  std::string rhs_marker = eval_stack.back();
  eval_stack.pop_back();

  if (!eval_stack.empty()) {
    std::string left_marker = eval_stack.back();
    eval_stack.pop_back();
    if (left_marker != "$str") {
      freeRegister(left_marker);
    }
  }

  if (auto *identifier =
          dynamic_cast<IdentifierExprNode<NodeInfo> *>(node.left.get())) {
    std::string var_name = identifier->identifier.getValue();
    if (isStringVariable(var_name)) {
      // RAX/RDX already hold the RHS string if rhs_marker == "$str"
      if (rhs_marker != "$str") {
        // If someone assigned an int to string (shouldn't happen if sema is
        // correct), coerce to empty
        loadStringLiteral("");
      }
      storeCurrentStringToVar(var_name);
      // For assignment expression value, leave string in RAX/RDX but don't push
      // marker to avoid accidental emission.
    } else {
      std::string var_location = formatSlot(*identifier);
      // rhs_marker contains a register name
      emitMove(var_location, rhs_marker);
      eval_stack.push_back(rhs_marker);
    }
  } else {
    LOG_WARN("[GEN] Unsupported assignmentExpr type");
  }
}

void CodeGenerator::visit(ASTNode<NodeInfo> &node) {
  ASTVisitor<NodeInfo>::visit(node);
}

void CodeGenerator::visit(UnaryExprNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited unary expr");
  std::string operand = eval_stack.back();
  eval_stack.pop_back();

  if (node.op.getType() == TokenType::TOKEN_MINUS) {
    emit("neg " + operand);
    eval_stack.push_back(operand);
  }
}

void CodeGenerator::visit(MethodCallNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited MethodCall");

  if (node.expr) node.expr->accept(*this);

  const size_t arg_count = node.arg_list.size();
  std::vector<std::string> arg_vals(arg_count);

  for (size_t rev = 0; rev < arg_count; ++rev) {
    size_t i = arg_count - 1 - rev;
    arg_vals[i] = eval_stack.back();
    eval_stack.pop_back();
  }

  std::vector<std::string> spilled;
  spill_live_regs(spilled);

  size_t reg_arg_count = std::min(arg_count, function_arg_registers.size());
  for (size_t i = 0; i < reg_arg_count; ++i) {
    if (arg_vals[i] == "$str") {
      LOG_ERROR("[GEN] String args not supported");
      return;
    }
    emit("push " + arg_vals[i]);
  }

  for (size_t rev = 0; rev < reg_arg_count; ++rev) {
    size_t i = reg_arg_count - 1 - rev;
    emit("pop " + function_arg_registers[i]);
  }

  std::string owner = node.extra.sym->owner_class.empty()
                          ? "<global>"
                          : node.extra.sym->owner_class;
  emitCall(method_symbols
               ->find_overload(owner, node.extra.sym->name,
                               node.extra.sym->param_types)
               ->name);
  eval_stack.push_back("rax");
}

void CodeGenerator::visit(ArgumentNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited ArgumentNode");
}

void CodeGenerator::visit(ParamNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited ParamNode");
}

void CodeGenerator::visit(ReturnStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited returnstmt");
  std::string marker = eval_stack.back();
  eval_stack.pop_back();

  if (marker != "$str") {
    emitMove("rax", marker);
  }
  emitReturn();
}

void CodeGenerator::visit(ClassNode<NodeInfo> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(FieldDeclNode<NodeInfo> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(MethodDeclNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Generating method: {}", node.identifier.getValue());
  (void)node;
}

void CodeGenerator::visit(ConstructorDeclNode<NodeInfo> &node) {
  // Stub: not implemented yet
}
void CodeGenerator::visit(ExprStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited exprstmt");
  (void)node;
  if (!eval_stack.empty() && eval_stack.back() == "$str") {
    emitWriteCurrentString();
    eval_stack.pop_back();
  }
}

void CodeGenerator::enter(BlockNode<NodeInfo> &) { enter_scope(); }

void CodeGenerator::exit(BlockNode<NodeInfo> &) { exit_scope(); }

void CodeGenerator::enter(IfStmtNode<NodeInfo> &node) {
  IfContext ctx;
  ctx.false_label = generateUniqueLabel("if_false");
  ctx.end_label = generateUniqueLabel("if_end");
  ctx.has_else = (node.else_stmt != nullptr);
  if_stack.push_back(ctx);
}

void CodeGenerator::before_else(IfStmtNode<NodeInfo> &) {
  if (if_stack.empty()) return;
  const auto &ctx = if_stack.back();
  emitJump(ctx.end_label);
  emitLabel(ctx.false_label);
}

void CodeGenerator::exit(IfStmtNode<NodeInfo> &) {
  if (if_stack.empty()) return;
  const auto &ctx = if_stack.back();
  if (ctx.has_else) {
    emitLabel(ctx.end_label);
  } else {
    emitLabel(ctx.false_label);
  }
  if_stack.pop_back();
}

void CodeGenerator::enter(WhileStmtNode<NodeInfo> &) {
  WhileContext ctx;
  ctx.start_label = generateUniqueLabel("loop_start");
  ctx.end_label = generateUniqueLabel("loop_end");
  while_stack.push_back(ctx);
  emitLabel(ctx.start_label);
}

void CodeGenerator::exit(WhileStmtNode<NodeInfo> &) {
  if (while_stack.empty()) return;
  const auto &ctx = while_stack.back();
  emitJump(ctx.start_label);
  emitLabel(ctx.end_label);
  while_stack.pop_back();
}

void CodeGenerator::enter(MethodDeclNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Generating method: {}", node.identifier.getValue());
  emitLabel(node.identifier.getValue());
  emit("push rbp");
  emitMove("rbp", "rsp");
  emit("sub rsp, " + std::to_string(node.extra.frame_size));
}

void CodeGenerator::exit(MethodDeclNode<NodeInfo> &) {}
