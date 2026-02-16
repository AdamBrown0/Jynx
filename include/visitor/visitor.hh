#ifndef VISITOR_H_
#define VISITOR_H_

#include <unordered_map>

#include "diagnostics.hh"
#include "log.hh"
#include "methodtable.hh"
#include "symbol.hh"
#include "token.hh"
#include "trie.hh"

template <typename Extra>
struct ASTNode;
template <typename Extra>
struct BinaryExprNode;
template <typename Extra>
struct UnaryExprNode;
template <typename Extra>
struct LiteralExprNode;
template <typename Extra>
struct IdentifierExprNode;
template <typename Extra>
struct AssignmentExprNode;
template <typename Extra>
struct MethodCallNode;
template <typename Extra>
struct ArgumentNode;
template <typename Extra>
struct ParamNode;
template <typename Extra>
struct ProgramNode;
template <typename Extra>
struct BlockNode;
template <typename Extra>
struct VarDeclNode;
template <typename Extra>
struct IfStmtNode;
template <typename Extra>
struct WhileStmtNode;
template <typename Extra>
struct ReturnStmtNode;
template <typename Extra>
struct ClassNode;
template <typename Extra>
struct FieldDeclNode;
template <typename Extra>
struct MethodDeclNode;
template <typename Extra>
struct ConstructorDeclNode;
template <typename Extra>
struct ExprStmtNode;

struct CompilerContext {
  std::unordered_map<std::string, Symbol> symbol_table;
  MethodTable method_table;
  KeywordTrie keywords;
};

template <typename Extra>
class ASTVisitor {
 public:
  ASTVisitor() = delete;
  explicit ASTVisitor(CompilerContext &ctx) : context(ctx) {}
  virtual ~ASTVisitor() = default;

  bool has_errors() const { return !errors.empty(); }
  const std::vector<std::string> &get_errors() const { return errors; }
  size_t error_count() const { return errors.size(); }

  virtual void enter(BinaryExprNode<Extra> &) {}
  virtual void enter(UnaryExprNode<Extra> &) {}
  virtual void enter(LiteralExprNode<Extra> &) {}
  virtual void enter(IdentifierExprNode<Extra> &) {}
  virtual void enter(AssignmentExprNode<Extra> &) {}
  virtual void enter(MethodCallNode<Extra> &) {}
  virtual void enter(ArgumentNode<Extra> &) {}
  virtual void enter(ParamNode<Extra> &) {}
  virtual void enter(ProgramNode<Extra> &) {}
  virtual void enter(BlockNode<Extra> &) {}
  virtual void enter(VarDeclNode<Extra> &) {}
  virtual void enter(IfStmtNode<Extra> &) {}
  virtual void enter(WhileStmtNode<Extra> &) {}
  virtual void enter(ReturnStmtNode<Extra> &) {}
  virtual void enter(ClassNode<Extra> &) {}
  virtual void enter(FieldDeclNode<Extra> &) {}
  virtual void enter(MethodDeclNode<Extra> &) {}
  virtual void enter(ConstructorDeclNode<Extra> &) {}
  virtual void enter(ExprStmtNode<Extra> &) {}

  virtual void before_else(IfStmtNode<Extra> &) {}

  virtual void exit(BinaryExprNode<Extra> &) {}
  virtual void exit(UnaryExprNode<Extra> &) {}
  virtual void exit(LiteralExprNode<Extra> &) {}
  virtual void exit(IdentifierExprNode<Extra> &) {}
  virtual void exit(AssignmentExprNode<Extra> &) {}
  virtual void exit(MethodCallNode<Extra> &) {}
  virtual void exit(ArgumentNode<Extra> &) {}
  virtual void exit(ParamNode<Extra> &) {}
  virtual void exit(ProgramNode<Extra> &) {}
  virtual void exit(BlockNode<Extra> &) {}
  virtual void exit(VarDeclNode<Extra> &) {}
  virtual void exit(IfStmtNode<Extra> &) {}
  virtual void exit(WhileStmtNode<Extra> &) {}
  virtual void exit(ReturnStmtNode<Extra> &) {}
  virtual void exit(ClassNode<Extra> &) {}
  virtual void exit(FieldDeclNode<Extra> &) {}
  virtual void exit(MethodDeclNode<Extra> &) {}
  virtual void exit(ConstructorDeclNode<Extra> &) {}
  virtual void exit(ExprStmtNode<Extra> &) {}

  virtual void visit(ASTNode<Extra> &) {}
  virtual void visit(BinaryExprNode<Extra> &) {}
  virtual void visit(UnaryExprNode<Extra> &) {}
  virtual void visit(LiteralExprNode<Extra> &) {}
  virtual void visit(IdentifierExprNode<Extra> &) {}
  virtual void visit(AssignmentExprNode<Extra> &) {}
  virtual void visit(MethodCallNode<Extra> &) {}
  virtual void visit(ArgumentNode<Extra> &) {}
  virtual void visit(ParamNode<Extra> &) {}
  virtual void visit(ProgramNode<Extra> &) {}
  virtual void visit(BlockNode<Extra> &) {}
  virtual void visit(VarDeclNode<Extra> &) {}
  virtual void visit(IfStmtNode<Extra> &) {}
  virtual void visit(WhileStmtNode<Extra> &) {}
  virtual void visit(ReturnStmtNode<Extra> &) {}
  virtual void visit(ClassNode<Extra> &) {}
  virtual void visit(FieldDeclNode<Extra> &) {}
  virtual void visit(MethodDeclNode<Extra> &) {}
  virtual void visit(ConstructorDeclNode<Extra> &) {}
  virtual void visit(ExprStmtNode<Extra> &) {}

 protected:
  CompilerContext &context;

  std::vector<std::unordered_map<std::string, Symbol>> scope_stack;
  std::vector<std::string> errors;

  std::string current_class;
  std::string current_method;
  TokenType current_method_ret_type = TokenType::TOKEN_UNKNOWN;

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

    if (!context.symbol_table.empty()) {
      auto global_found = context.symbol_table.find(name);
      if (global_found != context.symbol_table.end())
        return &global_found->second;
    }

    return nullptr;
  }

  Symbol *add_symbol(const Symbol &symbol) {
    if (!scope_stack.empty()) {
      scope_stack.back()[symbol.name] = symbol;
      return &scope_stack.back()[symbol.name];
    } else if (!context.symbol_table.empty()) {
      context.symbol_table[symbol.name] = symbol;
      return &context.symbol_table[symbol.name];
    }
    return nullptr;
  }

  bool check_symbol(const std::string &name) const {
    if (!scope_stack.empty()) {
      return scope_stack.back().find(name) != scope_stack.back().end();
    }
    if (context.symbol_table.empty()) return false;
    return context.symbol_table.find(name) != context.symbol_table.end();
  }

  void enter_class(const std::string &class_name) {
    current_class = class_name;
  }

  void exit_class() { current_class.clear(); }

  void enter_method(const std::string &method_name, TokenType ret_type) {
    current_method = method_name;
    current_method_ret_type = ret_type;
  }

  void exit_method() {
    current_method.clear();
    current_method_ret_type = TokenType::TOKEN_UNKNOWN;
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
