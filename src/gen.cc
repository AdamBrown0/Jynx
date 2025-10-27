#include "gen.hh"

#include <sstream>

#include "ast.hh"
#include "log.hh"

std::string CodeGenerator::generate(const ProgramNode<SemaExtra> &root) {
  LOG_DEBUG("[GEN] Resetting areas");
  // reset
  text_section.str("");
  text_section.clear();
  data_section.str("");
  data_section.clear();
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

  LOG_DEBUG("[GEN] Accepting nodes");
  auto &root_noconst = const_cast<ProgramNode<SemaExtra> &>(root);
  root_noconst.accept(*this);

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

  std::ostringstream out;
  out << text_section.str();
  // do data section for strings

  return out.str();
}

void CodeGenerator::visit(ProgramNode<SemaExtra> &node) {
  LOG_DEBUG("Code generator: Visiting ProgramNode with {} children",
            node.children.size());
  // nop
  // this is handled in the programnode struct
}

void CodeGenerator::visit(VarDeclNode<SemaExtra> &node) {
  std::string name = node.identifier.getValue();
  std::string loc = getVariableLocation(name);

  if (node.initializer) {
    node.initializer->accept(*this);
    std::string r = eval_stack.back();
    // eval_stack.pop_back();
    emitMove(loc, r);
    freeRegister(r);
  } else {
    emitMove(loc, "0");
  }
}

void CodeGenerator::visit(BinaryExprNode<SemaExtra> &node) {
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

void CodeGenerator::visit(LiteralExprNode<SemaExtra> &node) {
  if (node.literal_token.getType() == TokenType::TOKEN_INT) {
    std::string r = allocateRegister(true);
    if (r.empty()) r = "rax";
    emitMove(r, node.literal_token.getValue());
    eval_stack.push_back(r);
  } else {  // only support int for now
    // LOG_WARN("[GEN] Unsupported type: {}", node.literal_token.to_string());
    std::string r = allocateRegister(true);
    if (r.empty()) r = "rax";
    emit("xor " + r + ", " + r);
    eval_stack.push_back(r);
  }
}

void CodeGenerator::visit(IdentifierExprNode<SemaExtra> &node) {
  LOG_DEBUG("[GEN] Visited ident");

  std::string name = node.identifier.getValue();
  std::string var_location = getVariableLocation(name);
  std::string r = allocateRegister(true);
  if (r.empty()) r = "rax";

  emitMove(r, var_location);
  eval_stack.push_back(r);
}

void CodeGenerator::visit(IfStmtNode<SemaExtra> &node) {
  LOG_DEBUG("[GEN] Visited ifstmt");

  std::string false_label = generateUniqueLabel("if_false");
  std::string end_label = generateUniqueLabel("if_end");

  if (auto *cond =
          dynamic_cast<BinaryExprNode<SemaExtra> *>(node.condition.get())) {
    node.condition->accept(*this);

    switch (cond->op.getType()) {
      case TokenType::TOKEN_DEQ:
        emitConditionalJump("ne", false_label);
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

void CodeGenerator::visit(WhileStmtNode<SemaExtra> &node) {
  LOG_DEBUG("[GEN] Visited whilestmt");

  std::string start_label = generateUniqueLabel("loop_start");
  std::string end_label = generateUniqueLabel("loop_end");

  emitLabel(start_label);
  if (auto *cond =
          dynamic_cast<BinaryExprNode<SemaExtra> *>(node.condition.get())) {
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

void CodeGenerator::visit(BlockNode<SemaExtra> &node) {
  for (auto &stmt : node.statements) {
    stmt->accept(*this);
  }
}

void CodeGenerator::visit(AssignmentExprNode<SemaExtra> &node) {
  LOG_DEBUG("[GEN] Visited assignmentExpr");

  node.right->accept(*this);
  std::string right_reg = eval_stack.back();
  eval_stack.pop_back();

  if (auto *identifier =
          dynamic_cast<IdentifierExprNode<SemaExtra> *>(node.left.get())) {
    std::string var_name = identifier->identifier.getValue();
    std::string var_location = getVariableLocation(var_name);

    emitMove(var_location, right_reg);

    eval_stack.push_back(right_reg);
  } else {
    LOG_WARN("[GEN] Unsupported assignmentExpr type");
  }
}

// Additional required visitor stubs
void CodeGenerator::visit(ASTNode<SemaExtra> &node) {
  ASTVisitor<SemaExtra>::visit(node);
}

void CodeGenerator::visit(UnaryExprNode<SemaExtra> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(MethodCallNode<SemaExtra> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(ArgumentNode<SemaExtra> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(ParamNode<SemaExtra> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(ReturnStmtNode<SemaExtra> &node) {
  LOG_DEBUG("[GEN] Visited returnstmt");
  node.ret->accept(*this);
  std::string expr_reg = eval_stack.back();
  eval_stack.pop_back();

  emitMove("rax", expr_reg);
  emitReturn();
}

void CodeGenerator::visit(ClassNode<SemaExtra> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(FieldDeclNode<SemaExtra> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(MethodDeclNode<SemaExtra> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(ConstructorDeclNode<SemaExtra> &node) {
  // Stub: not implemented yet
}
void CodeGenerator::visit(ExprStmtNode<SemaExtra> &node) {
  LOG_DEBUG("[GEN] Visited exprstmt");
  if (node.expr) node.expr->accept(*this);
}
