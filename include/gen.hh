#ifndef GEN_H_
#define GEN_H_

#include <unistd.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "ast.hh"
#include "log.hh"
#include "token.hh"
#include "visitor/visitor.hh"

typedef struct Scope {
  int stack_offset_start;
  std::unordered_map<std::string, int> stack_offsets;
  std::unordered_map<std::string, std::pair<int, int>> string_var_slots;
} Scope;

class CodeGenerator : public ASTVisitor<NodeInfo> {
 public:
  using ASTVisitor<NodeInfo>::enter;
  using ASTVisitor<NodeInfo>::exit;
  using ASTVisitor<NodeInfo>::before_else;

  explicit CodeGenerator(CompilerContext &ctx) : ASTVisitor<NodeInfo>(ctx) {
    setupRegisters();
  }
  virtual ~CodeGenerator() = default;

  std::string generate(const ProgramNode<NodeInfo> &root);

  void visit(ASTNode<NodeInfo> &node) override;
  void visit(BinaryExprNode<NodeInfo> &node) override;
  void visit(UnaryExprNode<NodeInfo> &node) override;
  void visit(LiteralExprNode<NodeInfo> &node) override;
  void visit(IdentifierExprNode<NodeInfo> &node) override;
  void visit(AssignmentExprNode<NodeInfo> &node) override;
  void visit(MethodCallNode<NodeInfo> &node) override;
  void visit(ArgumentNode<NodeInfo> &node) override;
  void visit(ParamNode<NodeInfo> &node) override;
  void visit(ProgramNode<NodeInfo> &node) override;
  void visit(BlockNode<NodeInfo> &node) override;
  void visit(VarDeclNode<NodeInfo> &node) override;
  void visit(IfStmtNode<NodeInfo> &node) override;
  void visit(WhileStmtNode<NodeInfo> &node) override;
  void visit(ReturnStmtNode<NodeInfo> &node) override;
  void visit(ClassNode<NodeInfo> &node) override;
  void visit(FieldDeclNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(ConstructorDeclNode<NodeInfo> &node) override;
  void visit(ExprStmtNode<NodeInfo> &node) override;

  void enter(BlockNode<NodeInfo> &node) override;
  void exit(BlockNode<NodeInfo> &node) override;
  void enter(IfStmtNode<NodeInfo> &node) override;
  void before_else(IfStmtNode<NodeInfo> &node) override;
  void exit(IfStmtNode<NodeInfo> &node) override;
  void enter(WhileStmtNode<NodeInfo> &node) override;
  void exit(WhileStmtNode<NodeInfo> &node) override;
  void enter(MethodDeclNode<NodeInfo> &node) override;
  void exit(MethodDeclNode<NodeInfo> &node) override;

 private:
  struct IfContext {
    std::string false_label;
    std::string end_label;
    bool has_else;
  };

  struct WhileContext {
    std::string start_label;
    std::string end_label;
  };

  std::vector<IfContext> if_stack;
  std::vector<WhileContext> while_stack;

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
      throw std::runtime_error("No scope available for string variable: " +
                               name);
    }

    current_stack_offset += 16;
    int lenOffset = current_stack_offset;      // [rbp-lenOffset]
    int ptrOffset = current_stack_offset - 8;  // [rbp-ptrOffset]
    emit("sub rsp, 16");

    // Store in current scope
    scope_stack.back().string_var_slots[name] = {ptrOffset, lenOffset};
    return scope_stack.back().string_var_slots[name];
  }

  std::string allocateRegister(bool local, bool _32bit) {
    std::string r;
    std::vector<std::string> &regs =
        _32bit ? caller_saved_registers32 : caller_saved_registers64;
    if (local) {
      if (regs.empty()) return "";
      r = regs.back();
      regs.pop_back();
      live_regs.emplace(r);
      LOG_DEBUG("[gen] emplacing {}", r);
    } else {
      return "";
    }
    return r;
  }

  void freeRegister(const std::string &reg) {
    if (contains(caller_saved_registers_abi32, reg) &&
        !contains(caller_saved_registers32, reg)) {
      caller_saved_registers32.push_back(reg);
    } else if (contains(caller_saved_registers_abi64, reg) &&
               !contains(caller_saved_registers64, reg)) {
      caller_saved_registers64.push_back(reg);
    } else if (!contains(callee_saved_registers, reg)) {
      callee_saved_registers.push_back(reg);
    } else {
      LOG_FATAL("[GEN] Unknown register freed");
    }
    live_regs.erase(reg);
    LOG_DEBUG("[gen] erasing {}", reg);
  }

  TokenType builtin_type_name_to_type(std::string type_name) {
    if (!context.keywords.find(type_name)) return TokenType::TOKEN_UNKNOWN;

    if (type_name == "int") return TokenType::TOKEN_INT;
    if (type_name == "char") return TokenType::TOKEN_CHAR;

    return TokenType::TOKEN_UNKNOWN;
  }

  static inline std::string ptrType(int size) {
    if (size <= 8) return "byte ptr";
    if (size <= 16) return "word ptr";
    if (size <= 32) return "dword ptr";
    if (size <= 64) return "qword ptr";
    LOG_FATAL("128-bit sizes are not supported");
    return "";
  }

  static std::string formatSlotOffset(int offset) {
    if (offset > 0) return "+" + std::to_string(offset);
    return std::to_string(offset);
  }

  static inline std::string formatSlot(int offset) {
    return "[rbp-" + std::to_string(offset) + "]";
  }

  static inline std::string formatSlot(const VarDeclNode<NodeInfo> &node) {
    return "[rbp" + formatSlotOffset(node.extra.stack_offset) + "]";
  }

  static inline std::string formatSlot(
      const IdentifierExprNode<NodeInfo> &node) {
    return "[rbp" + formatSlotOffset(node.extra.stack_offset) + "]";
  }

  static inline std::string formatSlot(const ParamNode<NodeInfo> &node) {
    return "[rbp" + formatSlotOffset(node.extra.stack_offset) + "]";
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

    // Variable doesn't exist, allocate in current scope (like original
    // behavior)
    if (scope_stack.empty()) {
      throw std::runtime_error("No scope available for variable: " +
                               var.getValue());
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
  bool isStringVariable(const std::string &name) {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      if (it->string_var_slots.find(name) != it->string_var_slots.end()) {
        return true;
      }
    }
    return false;
  }

  // Allocate a new scalar variable in current scope (for declarations)
  std::string allocateVariableInCurrentScope(const std::string &name) {
    if (scope_stack.empty()) {
      throw std::runtime_error("No scope available for variable: " + name);
    }

    current_stack_offset += 8;  // 8-byte
    emit("sub rsp, 8");
    scope_stack.back().stack_offsets[name] = current_stack_offset;
    return formatSlot(current_stack_offset);
  }

  inline static bool contains(const std::vector<std::string> &v,
                              const std::string &item) {
    return std::find(v.begin(), v.end(), item) != v.end();
  }

  bool isCallerSaved(const std::string &r) {
    return contains(caller_saved_registers32, r) ||
           contains(caller_saved_registers64, r);
  }

  void spill_live_regs(std::vector<std::string> &spilled) {
    LOG_DEBUG("[gen] live regs count {}", live_regs.size());
    for (const auto &reg : live_regs) {
      if (reg == "rax" || !isCallerSaved(reg)) continue;
      emit("push " + reg);
      spilled.push_back(reg);
    }
  }

  void restore_spilled(std::vector<std::string> &spilled) {
    for (auto it = spilled.rbegin(); it != spilled.rend(); ++it)
      emit("pop " + *it);
  }

  void setupRegisters() {
    caller_saved_registers_abi64 = {"rax", "rcx", "rdx", "rsi", "rdi",
                                    "r8",  "r9",  "r10", "r11"};
    caller_saved_registers_abi32 = {"eax", "ecx", "edx",  "esi", "edi",
                                    "r8d", "r9d", "r10d", "r11d"};

    caller_saved_registers64 = {"rax", "rcx", "rdx", "rsi", "rdi",
                                "r8",  "r9",  "r10", "r11"};
    caller_saved_registers32 = {"eax", "ecx", "edx",  "esi", "edi",
                                "r8d", "r9d", "r10d", "r11d"};

    callee_saved_registers = {"rbx", "rbp", "r12", "r13", "r14", "r15"};

    function_arg_registers_abi64 = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    function_arg_registers_abi32 = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    function_arg_registers64 = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    function_arg_registers32 = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
  }

  std::stringstream text_section;
  std::stringstream rodata_section;

  std::vector<Scope> scope_stack;
  int current_stack_offset = 0;

  std::vector<std::string> caller_saved_registers_abi32;
  std::vector<std::string> caller_saved_registers_abi64;
  std::vector<std::string> function_arg_registers_abi32;
  std::vector<std::string> function_arg_registers_abi64;

  std::vector<std::string> caller_saved_registers32;
  std::vector<std::string> caller_saved_registers64;
  std::vector<std::string> callee_saved_registers;
  std::vector<std::string> function_arg_registers32;
  std::vector<std::string> function_arg_registers64;

  std::unordered_set<std::string> live_regs;
  std::unordered_map<std::string, int> spill_slots;

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
