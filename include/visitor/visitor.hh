#ifndef VISITOR_H_
#define VISITOR_H_

#include <unordered_map>

#include "ast.hh"
#include "log.hh"

struct Symbol {
  std::string name;
  TokenType type;
  SourceLocation decl_loc;

  std::string access_modifier = "private";
  bool is_static = false;

  std::string owner_class;

  std::vector<TokenType> param_types;
  std::vector<std::string> param_names;

  bool is_param = false;
  bool is_method = false;
  bool is_field = false;
  bool is_class = false;
};

template <typename Extra>
class ASTVisitor {
 public:
  ASTVisitor() = default;
  virtual ~ASTVisitor() = default;

  bool has_errors() const { return !errors.empty(); }
  const std::vector<std::string> &get_errors() const { return errors; }
  size_t error_count() const { return errors.size(); }

  virtual void visit(ASTNode<Extra> &node) {
    // Try to dispatch to concrete types
    if (auto *varDecl = dynamic_cast<VarDeclNode<Extra> *>(&node)) {
      visit(*varDecl);
    } else if (auto *program = dynamic_cast<ProgramNode<Extra> *>(&node)) {
      visit(*program);
    } else if (auto *binary = dynamic_cast<BinaryExprNode<Extra> *>(&node)) {
      visit(*binary);
    } else if (auto *block = dynamic_cast<BlockNode<Extra> *>(&node)) {
      visit(*block);
    } else if (auto *literal = dynamic_cast<LiteralExprNode<Extra> *>(&node)) {
      visit(*literal);
    } else if (auto *identifier =
                   dynamic_cast<IdentifierExprNode<Extra> *>(&node)) {
      visit(*identifier);
    } else if (auto *unary = dynamic_cast<UnaryExprNode<Extra> *>(&node)) {
      visit(*unary);
    } else if (auto *expr_stmt = dynamic_cast<ExprStmtNode<Extra> *>(&node)) {
      visit(*expr_stmt);
    } else if (auto *assignment =
                   dynamic_cast<AssignmentExprNode<Extra> *>(&node)) {
      visit(*assignment);
    } else if (auto *methodCall =
                   dynamic_cast<MethodCallNode<Extra> *>(&node)) {
      visit(*methodCall);
    } else if (auto *argument = dynamic_cast<ArgumentNode<Extra> *>(&node)) {
      visit(*argument);
    } else if (auto *param = dynamic_cast<ParamNode<Extra> *>(&node)) {
      visit(*param);
    } else if (auto *ifStmt = dynamic_cast<IfStmtNode<Extra> *>(&node)) {
      visit(*ifStmt);
    } else if (auto *whileStmt = dynamic_cast<WhileStmtNode<Extra> *>(&node)) {
      visit(*whileStmt);
    } else if (auto *returnStmt =
                   dynamic_cast<ReturnStmtNode<Extra> *>(&node)) {
      visit(*returnStmt);
    } else if (auto *classNode = dynamic_cast<ClassNode<Extra> *>(&node)) {
      visit(*classNode);
    } else if (auto *fieldDecl = dynamic_cast<FieldDeclNode<Extra> *>(&node)) {
      visit(*fieldDecl);
    } else if (auto *methodDecl =
                   dynamic_cast<MethodDeclNode<Extra> *>(&node)) {
      visit(*methodDecl);
    } else if (auto *ctorDecl =
                   dynamic_cast<ConstructorDeclNode<Extra> *>(&node)) {
      visit(*ctorDecl);
    } else {
      LOG_DEBUG("No specific dispatch found for type: {}", typeid(node).name());
    }
  };
  virtual void visit(BinaryExprNode<Extra> &node) = 0;
  virtual void visit(UnaryExprNode<Extra> &node) = 0;
  virtual void visit(LiteralExprNode<Extra> &node) = 0;
  virtual void visit(IdentifierExprNode<Extra> &node) = 0;
  virtual void visit(AssignmentExprNode<Extra> &node) = 0;
  virtual void visit(MethodCallNode<Extra> &node) = 0;
  virtual void visit(ArgumentNode<Extra> &node) = 0;
  virtual void visit(ParamNode<Extra> &node) = 0;
  virtual void visit(ProgramNode<Extra> &node) = 0;
  virtual void visit(BlockNode<Extra> &node) = 0;
  virtual void visit(VarDeclNode<Extra> &node) = 0;
  virtual void visit(IfStmtNode<Extra> &node) = 0;
  virtual void visit(WhileStmtNode<Extra> &node) = 0;
  virtual void visit(ReturnStmtNode<Extra> &node) = 0;
  virtual void visit(ClassNode<Extra> &node) = 0;
  virtual void visit(FieldDeclNode<Extra> &node) = 0;
  virtual void visit(MethodDeclNode<Extra> &node) = 0;
  virtual void visit(ConstructorDeclNode<Extra> &node) = 0;
  virtual void visit(ExprStmtNode<Extra> &node) = 0;

 protected:
  std::unordered_map<std::string, Symbol> global_symbols;
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

    auto global_found = global_symbols.find(name);
    if (global_found != global_symbols.end()) return &global_found->second;

    return nullptr;
  }

  void add_symbol(const Symbol &symbol) {
    if (!scope_stack.empty()) {
      scope_stack.back()[symbol.name] = symbol;
    } else {
      global_symbols[symbol.name] = symbol;
    }
  }

  bool check_symbol(const std::string &name) const {
    if (!scope_stack.empty()) {
      return scope_stack.back().find(name) != scope_stack.back().end();
    }
    return global_symbols.find(name) != global_symbols.end();
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
  }

  void report_error(const std::string &message, int line, int col) {
    errors.push_back(message);
    Log::Compiler::semantic_error(message, line, col);
  }
};

#endif  // VISITOR_H_
