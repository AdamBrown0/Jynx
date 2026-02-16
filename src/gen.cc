#include "gen.hh"

#include <cassert>
#include <sstream>
#include <string>

#include "ast.hh"
#include "log.hh"
#include "token.hh"

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
  emitCall("global_main_");
  emit("movsx rdi, eax");
  // emitMove("rdi", "eax");
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
    std::string loc =
        ptrType(token_type_to_bit_size(node.extra.resolved_type)) + " " +
        formatSlot(node);
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
  if (token_implicit_cast(node.literal_token.getType(), TokenType::TOKEN_INT)) {
    eval_stack.push_back(node.literal_token.getValue());
  } else if (node.literal_token.getType() == TokenType::TOKEN_STRING) {
    // Load string literal into RAX (ptr) and RDX (len)
    loadStringLiteral(node.literal_token.getValue());
    eval_stack.push_back("$str");
  } else {  // only support int for now
    LOG_WARN("[GEN] Unsupported type: {}", node.literal_token.to_string());
    std::string r = allocateRegister(true, false);
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
    int bit_size = token_type_to_bit_size(node.extra.resolved_type);
    std::string var_location = ptrType(bit_size) + " " + formatSlot(node);
    std::string r = allocateRegister(true, bit_size <= 32);
    if (r.empty()) r = "rax";
    if (bit_size <= 32 && !contains(function_arg_registers_abi32, r)) {
      emit("movsx " + r + ", " + var_location);
    }

    else {
      emitMove(r, var_location);
      LOG_FATAL("IdentifierExprNode errored");
    }
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

  size_t arg_count = node.arg_list.size();
  std::vector<std::string> arg_vals;
  for (size_t i = 0; i < arg_count; ++i) {
    arg_vals.push_back(eval_stack.back());
    eval_stack.pop_back();
  }
  std::reverse(arg_vals.begin(), arg_vals.end());

  std::vector<std::string> spilled;
  spill_live_regs(spilled);

  size_t reg_arg_count =
      std::min(arg_count, function_arg_registers_abi32.size());
  size_t stack_arg_count =
      arg_count > reg_arg_count ? arg_count - reg_arg_count : 0;

  for (size_t i = 0; i < stack_arg_count; ++i) {
    LOG_DEBUG("arg_vals.size {}, arg_vals.size() - i {}", arg_vals.size(),

              arg_vals.size() - i);
    emit("push " + arg_vals[arg_vals.size() - i - 1]);
  }

  size_t stack_bytes = stack_arg_count * 8;
  // if ((stack_bytes % 16) != 0) {
  //   emit("sub rsp, 8");  // align
  //   stack_bytes += 8;
  // }

  std::vector<std::string> used_function_arg_regs;

  for (size_t i = reg_arg_count; i-- > 0;) {
    if (token_type_to_bit_size(node.arg_list[i]->extra.sym->type) <= 32) {
      emitMove(function_arg_registers32[i], arg_vals[i]);
      used_function_arg_regs.push_back(function_arg_registers32[i]);
    } else {
      emitMove(function_arg_registers64[i], arg_vals[i]);
      used_function_arg_regs.push_back(function_arg_registers64[i]);
    }
    freeRegister(arg_vals[i]);
  }

  // uhh currently only static, if instance use rdi reg

  std::string owner = node.extra.sym->owner_class.empty()
                          ? "global"
                          : node.extra.sym->owner_class;

  const Symbol *overload = context.method_table.find_overload(
      owner, node.extra.sym->name, node.extra.sym->param_types);

  emitCall(overload->method_key);

  if (overload->type != TokenType::TOKEN_UNKNOWN) {
    int bit_size =
        token_type_to_bit_size(builtin_type_name_to_type(overload->type_name));
    std::string reg = allocateRegister(true, bit_size <= 32);
    emitMove(reg, bit_size <= 32 ? "eax" : "rax");
    eval_stack.push_back(reg);
  }

  if (stack_bytes > 0) emit("add rsp, " + std::to_string(stack_bytes));

  for (auto reg : used_function_arg_regs) freeRegister(reg);

  restore_spilled(spilled);

  // free all registers args took

  // eval_stack.push_back("rax");
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
    emitMove(token_type_to_bit_size(
                 builtin_type_name_to_type(node.ret->extra.type_name)) <= 32
                 ? "eax"
                 : "rax",
             marker);
  }
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
  emitLabel(node.extra.sym->method_key);
  emit("push rbp");
  emitMove("rbp", "rsp");
  emit("sub rsp, " + std::to_string(node.extra.frame_size));

  for (size_t i = 0;
       i < node.param_list.size() && i < function_arg_registers_abi32.size();
       ++i) {
    LOG_DEBUG("bit size: {}",
              token_type_to_bit_size(node.param_list[i]->extra.sym->type));
    int bit_size = token_type_to_bit_size(
        builtin_type_name_to_type(node.param_list[i]->extra.sym->type_name));
    std::string param_reg = bit_size <= 32 ? function_arg_registers32[i]
                                           : function_arg_registers64[i];
    std::string local_slot =
        ptrType(bit_size) + " " + formatSlot(*node.param_list[i]);
    emitMove(local_slot, param_reg);
  }
}

void CodeGenerator::exit(MethodDeclNode<NodeInfo> &) {
  // emit("pop rbp");
  emitReturn();
}
