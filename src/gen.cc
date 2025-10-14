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
  if (!eval_stack.empty()) {
    std::string top = eval_stack.back();
    eval_stack.pop_back();
    emitMove("rax", top);
    freeRegister(top);
  } else {
    emitMove("rax", "0");
  }

  LOG_DEBUG("[GEN] Leaving");
  // leave
  emit("leave");
  emit("ret\n");

  // generate entrypoint
  emitLabel("_start");
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

  emitArithmetic(node.op.getType(), left, right, dest);
  freeRegister(right);
  eval_stack.push_back(left);
}

void CodeGenerator::visit(LiteralExprNode<SemaExtra> &node) {
  if (node.literal_token.getType() == TokenType::TOKEN_INT) {
    std::string r = allocateRegister(true);
    if (r.empty()) r = "rax";
    emitMove(r, node.literal_token.getValue());
    eval_stack.push_back(r);
  } else {  // only support int for now
    LOG_WARN("[GEN] Unsupported type");
    std::string r = allocateRegister(true);
    if (r.empty()) r = "rax";
    emit("xor" + r + ", " + r);
    eval_stack.push_back(r);
  }
}

void CodeGenerator::visit(IdentifierExprNode<SemaExtra> &node) {
  // nop
}

void CodeGenerator::visit(IfStmtNode<SemaExtra> &node) {
  // Stub: not implemented yet
}

void CodeGenerator::visit(BlockNode<SemaExtra> &node) {
  for (auto &stmt : node.statements) {
    stmt->accept(*this);
  }
}

void CodeGenerator::visit(AssignmentExprNode<SemaExtra> &node) {
  // Stub: not implemented yet
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
  // Stub: not implemented yet
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
void CodeGenerator::visit(ExprStmtNode<SemaExtra> &node) {}
