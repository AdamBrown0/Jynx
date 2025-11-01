#ifndef GEN_H_
#define GEN_H_

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "ast.hh"
#include "log.hh"
#include "visitor/visitor.hh"

typedef struct Scope {
  int stack_offset_start;
  std::unordered_map<std::string, int> stack_offsets;
  std::unordered_map<std::string, std::pair<int, int>> string_var_slots;
} Scope;

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
  void visit(WhileStmtNode<SemaExtra> &node) override;
  void visit(ReturnStmtNode<SemaExtra> &node) override;
  void visit(ClassNode<SemaExtra> &node) override;
  void visit(FieldDeclNode<SemaExtra> &node) override;
  void visit(MethodDeclNode<SemaExtra> &node) override;
  void visit(ConstructorDeclNode<SemaExtra> &node) override;
  void visit(ExprStmtNode<SemaExtra> &node) override;

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
  void emitCompare(const std::string &left, const std::string &right) {
    emit("cmp " + left + ", " + right);
  }
  void emitJump(const std::string &label) { emit("jmp " + label); }
  void emitConditionalJump(const std::string &condition,
                           const std::string &label) {
    emit("j" + condition + " " + label);
  }
  void emitCall(const std::string &function) { emit("call " + function); }
  void emitReturn() {
    emit("leave");
    emit("ret");
  }
  void emitLabel(const std::string &label) { text_section << label << ":\n"; }

  // Emit a string literal into the rodata pool (deduplicated).
  // contents are assumed to be already lexer-unescaped.
  void poolStringLiteral(const std::string &contents) {
    if (literal_pool_labels.find(contents) != literal_pool_labels.end()) return;
    std::string label = generateLiteralLabel();
    literal_pool_labels[contents] = label;
    literal_pool_emission.emplace_back(label, contents);
  }

  // Write a single literal into the rodata section stream.
  void emitRodataLiteral(const std::string &label,
                         const std::string &contents) {
    rodata_section << label << ":\n";
    rodata_section << "   .ascii \"" << contents << "\"\n";
    rodata_section << "   .byte 0\n";
  }

  // Print the string currently in RAX (ptr) and RDX (len) via Linux
  // write(1,...).
  void emitWriteCurrentString() {
    // Preserve ptr from rax by moving to rsi as required by write syscall.
    emit("mov rsi, rax");  // buf
    emit("mov rax, 1");    // sys_write
    emit("mov rdi, 1");    // fd = stdout
    // rdx already holds len
    emit("syscall");
  }

  // Allocate 16 bytes on the stack for a string descriptor (ptr,len) for
  // variable 'name'. Returns pair of offsets: {ptrOffset, lenOffset} relative
  // to rbp (positive integers used as [rbp-offset]).
  std::pair<int, int> ensureStringVarSlots(const std::string &name) {
    // First check if variable already exists in any scope
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      auto found = it->string_var_slots.find(name);
      if (found != it->string_var_slots.end()) {
        return found->second;
      }
    }
    
    // Variable doesn't exist, allocate in current (innermost) scope
    if (scope_stack.empty()) {
      throw std::runtime_error("No scope available for string variable: " + name);
    }
    
    current_stack_offset += 16;
    int lenOffset = current_stack_offset;      // [rbp-lenOffset]
    int ptrOffset = current_stack_offset - 8;  // [rbp-ptrOffset]
    emit("sub rsp, 16");
    
    // Store in current scope
    scope_stack.back().string_var_slots[name] = {ptrOffset, lenOffset};
    return scope_stack.back().string_var_slots[name];
  }

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
                  reg) == caller_saved_registers.end()) {
      caller_saved_registers.push_back(reg);
    } else {
      callee_saved_registers.push_back(reg);
    }
  }

  static inline std::string formatSlot(int offset) {
    return "[rbp-" + std::to_string(offset) + "]";
  }

  static inline std::string formatStringLabel(std::string label) {
    return "[rip+" + label + "]";
  }

  // Load a string literal into RAX (ptr) and RDX (len). Ensures the literal
  // exists in rodata pool.
  void loadStringLiteral(const std::string &contents) {
    poolStringLiteral(contents);
    const std::string &label = literal_pool_labels[contents];
    emit("lea rax, " + formatStringLabel(label));
    emit("mov rdx, " + std::to_string(contents.size()));
  }

  // Store the current string in RAX/RDX to a named local variable slots.
  void storeCurrentStringToVar(const std::string &name) {
    auto [ptrOff, lenOff] = ensureStringVarSlots(name);
    emit("mov QWORD PTR " + formatSlot(ptrOff) + ", rax");
    emit("mov QWORD PTR " + formatSlot(lenOff) + ", rdx");
  }

  // Load a string from a named local variable slots into RAX/RDX.
  void loadStringFromVar(const std::string &name) {
    // Search for string variable in scopes
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      auto found = it->string_var_slots.find(name);
      if (found != it->string_var_slots.end()) {
        auto [ptrOff, lenOff] = found->second;
        emit("mov rax, QWORD PTR " + formatSlot(ptrOff));
        emit("mov rdx, QWORD PTR " + formatSlot(lenOff));
        return;
      }
    }
    
    // If not found, treat as empty string (or could throw error)
    loadStringLiteral("");
  }

  std::string getVariableLocation(const Token &var) {
    // First check if variable already exists in any scope
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      auto found = it->stack_offsets.find(var.getValue());
      if (found != it->stack_offsets.end()) {
        return formatSlot(found->second);
      }
    }

    // Variable doesn't exist, allocate in current scope (like original behavior)
    if (scope_stack.empty()) {
      throw std::runtime_error("No scope available for variable: " + var.getValue());
    }
    
    current_stack_offset += 8;  // 8-byte
    emit("sub rsp, 8");
    scope_stack.back().stack_offsets[var.getValue()] = current_stack_offset;
    return formatSlot(current_stack_offset);
  }

  void enter_scope() {
    scope_stack.emplace_back();
    scope_stack.back().stack_offset_start = current_stack_offset;
  }

  void exit_scope() {
    if (scope_stack.empty()) return;

    int bytes_allocated =
        current_stack_offset - scope_stack.back().stack_offset_start;
    if (bytes_allocated > 0) {
      emit("add rsp, " + std::to_string(bytes_allocated));
      current_stack_offset = scope_stack.back().stack_offset_start;
    }

    scope_stack.pop_back();
  }

  // Check if a variable is a string type by searching scopes
  bool isStringVariable(const std::string& name) {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      if (it->string_var_slots.find(name) != it->string_var_slots.end()) {
        return true;
      }
    }
    return false;
  }

  // Allocate a new scalar variable in current scope (for declarations)
  std::string allocateVariableInCurrentScope(const std::string& name) {
    if (scope_stack.empty()) {
      throw std::runtime_error("No scope available for variable: " + name);
    }
    
    current_stack_offset += 8;  // 8-byte
    emit("sub rsp, 8");
    scope_stack.back().stack_offsets[name] = current_stack_offset;
    return formatSlot(current_stack_offset);
  }

  void setupRegisters() {
    caller_saved_registers = {"rax", "rcx", "rdx", "rsi", "rdi",
                              "r8",  "r9",  "r10", "r11"};

    callee_saved_registers = {"rbx", "rbp", "r12", "r13", "r14", "r15"};
  }

  std::stringstream text_section;
  std::stringstream rodata_section;

  std::vector<Scope> scope_stack;
  int current_stack_offset = 0;

  //   std::vector<std::string> available_registers;
  std::vector<std::string> caller_saved_registers;
  std::vector<std::string> callee_saved_registers;
  // REMOVED: std::unordered_map<std::string, int> stack_offsets;
  // REMOVED: std::unordered_map<std::string, std::pair<int, int>> string_var_slots;
  // Literal pool: content -> label, plus ordered emission list
  std::unordered_map<std::string, std::string> literal_pool_labels;
  std::vector<std::pair<std::string, std::string>> literal_pool_emission;
  std::vector<std::string> eval_stack;
  int label_counter = 0;

  inline std::string generateUniqueLabel(const std::string &prefix = "L") {
    return prefix + std::to_string(++label_counter);
  }

  inline std::string generateLiteralLabel() {
    return generateUniqueLabel(".LC");
  }

  bool isComparisonOp(TokenType op) {
    switch (op) {
      case TokenType::TOKEN_DEQ:
      case TokenType::TOKEN_NEQ:
      case TokenType::TOKEN_GEQ:
      case TokenType::TOKEN_GT:
      case TokenType::TOKEN_LEQ:
      case TokenType::TOKEN_LT:
        return true;
      default:
        return false;
    }
  }
};

#endif  // GEN_H_
