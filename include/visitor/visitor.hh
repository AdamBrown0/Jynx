#ifndef VISITOR_H_
#define VISITOR_H_

#include <unordered_map>

#include "context.hh"
#include "diagnostics.hh"
#include "log.hh"
#include "symbol.hh"
#include "token.hh"

struct ASTNode;

struct BinaryExprNode;

struct UnaryExprNode;

struct LiteralExprNode;

struct IdentifierExprNode;

struct AssignmentExprNode;

struct MethodCallNode;

struct ArgumentNode;

struct ParamNode;

struct ProgramNode;

struct BlockNode;

struct VarDeclNode;

struct IfStmtNode;

struct WhileStmtNode;

struct ReturnStmtNode;

struct ClassNode;

struct FieldDeclNode;

struct MethodDeclNode;

struct ConstructorDeclNode;

struct ExprStmtNode;

class ASTVisitor {
 public:
  ASTVisitor() = delete;
  explicit ASTVisitor(CompilerContext &ctx) : ctx(ctx) {}
  virtual ~ASTVisitor() = default;

  bool has_errors() const { return !errors.empty(); }
  const std::vector<std::string> &get_errors() const { return errors; }
  size_t error_count() const { return errors.size(); }

 protected:
  CompilerContext &ctx;

  std::vector<std::unordered_map<std::string, Symbol>> scope_stack;
  std::vector<std::string> errors;

  std::string current_class;
  std::string current_method;
  std::string current_method_ret_type;

  void push_scope() { scope_stack.emplace_back(); }
  void pop_scope() {
    if (!scope_stack.empty()) {
      scope_stack.pop_back();
    }
  }

  Symbol *lookup_symbol(const std::string &name) {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      auto found = it->find(name);
      if (found != it->end()) {
        return &found->second;
      }
    }

    if (!ctx.symbol_table.empty()) {
      auto global_found = ctx.symbol_table.find(name);
      if (global_found != ctx.symbol_table.end()) return &global_found->second;
    }

    return nullptr;
  }

  TokenType builtin_type_name_to_type(std::string type_name) {
    if (type_name.find("[")) return TokenType::TOKEN_ARRAY;

    if (!ctx.keywords.find(type_name)) return TokenType::TOKEN_UNKNOWN;

    if (type_name == "int") return TokenType::TOKEN_INT;
    if (type_name == "char") return TokenType::TOKEN_CHAR;

    return TokenType::TOKEN_UNKNOWN;
  }

  Symbol *add_symbol(const Symbol &symbol) {
    if (!scope_stack.empty()) {
      scope_stack.back()[symbol.name] = symbol;
      return &scope_stack.back()[symbol.name];
    } else if (!ctx.symbol_table.empty()) {
      ctx.symbol_table[symbol.name] = symbol;
      return &ctx.symbol_table[symbol.name];
    }
    return nullptr;
  }

  bool check_symbol(const std::string &name) const {
    if (!scope_stack.empty()) {
      return scope_stack.back().find(name) != scope_stack.back().end();
    }
    if (ctx.symbol_table.empty()) return false;
    return ctx.symbol_table.find(name) != ctx.symbol_table.end();
  }

  void report_error(const std::string &message, SourceLocation loc) {
    errors.push_back(message);
    Log::Compiler::semantic_error(message, loc.line, loc.col);
    Diagnostics::instance().report_error(message);
  }

  void report_error(const std::string &message, int line, int col) {
    errors.push_back(message);
    Log::Compiler::semantic_error(message, line, col);
    Diagnostics::instance().report_error(message);
  }
};

#endif  // VISITOR_H_
