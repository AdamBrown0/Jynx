#ifndef GEN_H_
#define GEN_H_

#include <algorithm>
#include <string>

#include "ast.hh"
#include "visitor/visitor.hh"

class CodeGenerator : public ASTVisitor<SemaExtra> {
 public:
  CodeGenerator() { setupRegisters(); }
  virtual ~CodeGenerator() = default;

  std::string generate(const ProgramNode<SemaExtra> &root);

  void visit(ASTNode<SemaExtra> &node) override;
  void visit(BinaryExprNode<SemaExtra> &node) override;
  void visit(UnaryExprNode<SemaExtra> &node) override;
  void visit(LiteralExprNode<SemaExtra> &node) override;
  void visit(IdentifierExprNode<SemaExtra> &node) override;
  void visit(AssignmentExprNode<SemaExtra> &node) override;
  void visit(MethodCallNode<SemaExtra> &node) override;
  void visit(ArgumentNode<SemaExtra> &node) override;
  void visit(ParamNode<SemaExtra> &node) override;
  void visit(ProgramNode<SemaExtra> &node) override;
  void visit(BlockNode<SemaExtra> &node) override;
  void visit(VarDeclNode<SemaExtra> &node) override;
  void visit(IfStmtNode<SemaExtra> &node) override;
  void visit(ReturnStmtNode<SemaExtra> &node) override;
  void visit(ClassNode<SemaExtra> &node) override;
  void visit(FieldDeclNode<SemaExtra> &node) override;
  void visit(MethodDeclNode<SemaExtra> &node) override;
  void visit(ConstructorDeclNode<SemaExtra> &node) override;

 private:
  void emit(const std::string &instruction) {
    text_section << "    " << instruction << "\n";
  }
  void emitMove(const std::string &dst, const std::string &src) {
    emit("mov " + dst + ", " + src);
  }
  void emitArithmetic(const TokenType op, const std::string &left,
                      const std::string &right, const std::string &dest) {
    if (dest != left) emitMove(left, dest);

    switch (op) {
      case TokenType::TOKEN_PLUS:
        emit("add " + dest + ", " + right);
        break;
      case TokenType::TOKEN_MINUS:
        emit("sub " + dest + ", " + right);
        break;
      case TokenType::TOKEN_MULTIPLY:
        emit("imul " + dest + ", " + right);
        break;
      case TokenType::TOKEN_DIVIDE:
        if (dest != "rax") emitMove("rax", dest);
        emit("cqo");
        emit("idiv " + right);
        if (dest != "rax" && dest != "rdx") emitMove(dest, "rax");
        break;
      default:
        break;
    }
  }
  void emitCompare(const std::string &left, const std::string &right);
  void emitJump(const std::string &label);
  void emitConditionalJump(const std::string &condition,
                           const std::string &label);
  void emitCall(const std::string &function) { emit("call " + function); }
  void emitReturn();
  void emitLabel(const std::string &label) { text_section << label << ":\n"; }

  std::string allocateRegister(bool local) {
    std::string r;
    if (local) {
      if (caller_saved_registers.empty()) return "";
      r = caller_saved_registers.back();
      caller_saved_registers.pop_back();
    } else {
      return "";
    }
    return r;
  }

  void freeRegister(const std::string &reg) {
    if (std::find(caller_saved_registers.begin(), caller_saved_registers.end(),
                  reg) != caller_saved_registers.end()) {
      caller_saved_registers.push_back(reg);
    } else {
      callee_saved_registers.push_back(reg);
    }
  }

  static inline std::string formatSlot(int offset) {
    return "[rbp-" + std::to_string(offset) + "]";
  }

  std::string getVariableLocation(const std::string &name) {
    auto it = stack_offsets.find(name);
    if (it == stack_offsets.end()) {
      current_stack_offset += 8;  // 8-byte
      stack_offsets[name] = current_stack_offset;
      emit("sub rsp, 8");
      it = stack_offsets.find(name);
    }

    return formatSlot(it->second);
  }

  void setupRegisters() {
    caller_saved_registers = {"rax", "rcx", "rdx", "rsi", "rdi",
                              "r8",  "r9",  "r10", "r11"};

    callee_saved_registers = {"rbx", "rbp", "r12", "r13", "r14", "r15"};
  }

  std::stringstream text_section;
  std::stringstream data_section;

  //   std::vector<std::string> available_registers;
  std::vector<std::string> caller_saved_registers;
  std::vector<std::string> callee_saved_registers;
  std::unordered_map<std::string, int> stack_offsets;
  std::vector<std::string> eval_stack;
  int current_stack_offset = 0;
  int label_counter = 0;

  std::string generateUniqueLabel(const std::string &prefix = "L");
};

#endif  // GEN_H_
