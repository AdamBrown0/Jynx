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
  emitLabel("_jynx_main");
  emit("push rbp");
  emit("mov rbp, rsp\n");

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
  emitCall("_jynx_main");
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
      node.initializer->accept(*this);  // expect string in RAX/RDX
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
        allocateVariableInCurrentScope(node.identifier.getValue());
    if (node.initializer) {
      node.initializer->accept(*this);
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
  node.left->accept(*this);
  node.right->accept(*this);

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
    std::string var_location = getVariableLocation(node.identifier);
    std::string r = allocateRegister(true);
    if (r.empty()) r = "rax";
    emitMove(r, var_location);
    eval_stack.push_back(r);
  }
}

void CodeGenerator::visit(IfStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited ifstmt");

  std::string false_label = generateUniqueLabel("if_false");
  std::string end_label = generateUniqueLabel("if_end");

  if (auto *cond =
          dynamic_cast<BinaryExprNode<NodeInfo> *>(node.condition.get())) {
    node.condition->accept(*this);

    switch (cond->op.getType()) {
      case TokenType::TOKEN_DEQ:
        emitConditionalJump("ne", false_label);
        break;
      case TokenType::TOKEN_NEQ:
        emitConditionalJump("e", false_label);
        break;
      case TokenType::TOKEN_GEQ:
        emitConditionalJump("l", false_label);
        break;
      case TokenType::TOKEN_GT:
        emitConditionalJump("le", false_label);
        break;
      case TokenType::TOKEN_LEQ:
        emitConditionalJump("g", false_label);
        break;
      case TokenType::TOKEN_LT:
        emitConditionalJump("ge", false_label);
        break;
      default:
        LOG_ERROR("[GEN] Expected conditional operator, found {}",
                  cond->op.to_string());
    }
  }

  node.statement->accept(*this);

  if (node.else_stmt) {
    emitJump(end_label);
  }

  emitLabel(false_label);
  if (node.else_stmt) {
    node.else_stmt->accept(*this);
    emitLabel(end_label);
  }
}

void CodeGenerator::visit(WhileStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited whilestmt");

  std::string start_label = generateUniqueLabel("loop_start");
  std::string end_label = generateUniqueLabel("loop_end");

  emitLabel(start_label);
  if (auto *cond =
          dynamic_cast<BinaryExprNode<NodeInfo> *>(node.condition.get())) {
    node.condition->accept(*this);

    switch (cond->op.getType()) {
      case TokenType::TOKEN_DEQ:
        emitConditionalJump("ne", end_label);
        break;
      case TokenType::TOKEN_GEQ:
        emitConditionalJump("l", end_label);
        break;
      case TokenType::TOKEN_GT:
        emitConditionalJump("le", end_label);
        break;
      case TokenType::TOKEN_LEQ:
        emitConditionalJump("g", end_label);
        break;
      case TokenType::TOKEN_LT:
        emitConditionalJump("ge", end_label);
        break;
      default:
        LOG_ERROR("[GEN] Expected conditional operator, found {}",
                  cond->op.to_string());
    }
  }

  node.statement->accept(*this);
  emitJump(start_label);
  emitLabel(end_label);
}

void CodeGenerator::visit(BlockNode<NodeInfo> &node) {
  enter_scope();
  for (auto &stmt : node.statements) {
    stmt->accept(*this);
  }
  exit_scope();
}

void CodeGenerator::visit(AssignmentExprNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited assignmentExpr");

  node.right->accept(*this);
  std::string rhs_marker = eval_stack.back();
  eval_stack.pop_back();

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
      std::string var_location = getVariableLocation(identifier->identifier);
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

  node.operand->accept(*this);
  std::string operand = eval_stack.back();
  eval_stack.pop_back();

  if (node.op.getType() == TokenType::TOKEN_MINUS) {
    emit("neg " + operand);
    eval_stack.push_back(operand);
  }
}

void CodeGenerator::visit(MethodCallNode<NodeInfo> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(ArgumentNode<NodeInfo> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(ParamNode<NodeInfo> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(ReturnStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited returnstmt");
  node.ret->accept(*this);
  std::string marker = eval_stack.back();
  eval_stack.pop_back();

  if (marker == "$str") {
    // RAX/RDX already contain the string descriptor by convention
    // Quick: print on return for demo
    emitWriteCurrentString();
  } else {
    // Scalar return: move to rax
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
}

void CodeGenerator::visit(ConstructorDeclNode<NodeInfo> &node) {
  // Stub: not implemented yet
}
void CodeGenerator::visit(ExprStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[GEN] Visited exprstmt");
  if (node.expr) {
    node.expr->accept(*this);
    // If the top of eval stack denotes a string, print it (demo)
    if (!eval_stack.empty() && eval_stack.back() == "$str") {
      emitWriteCurrentString();
      eval_stack.pop_back();
    }
  }
}
