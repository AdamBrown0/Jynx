#ifndef TYPECHECKER_H_
#define TYPECHECKER_H_

#include <unordered_map>

#include "ast.hh"
#include "methodtable.hh"
#include "visitor.hh"

typedef struct TypeInfo {
  TokenType token_type;
  std::string type_name;
} TypeInfo;

class TypeCheckerVisitor : public ASTVisitor<NodeInfo> {
 public:
  TypeCheckerVisitor() {}

  TypeCheckerVisitor(std::unordered_map<std::string, Symbol> &symbols,
                     MethodTable &methods) {
    set_global_symbols(&symbols);
    set_method_table(&methods);
  }

  void visit(BinaryExprNode<NodeInfo> &node) override;
  void visit(UnaryExprNode<NodeInfo> &node) override;
  void visit(LiteralExprNode<NodeInfo> &node) override;
  void visit(IdentifierExprNode<NodeInfo> &node) override;
  void visit(AssignmentExprNode<NodeInfo> &node) override;
  void visit(VarDeclNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(MethodCallNode<NodeInfo> &node) override;
  void visit(ExprStmtNode<NodeInfo> &node) override;
  void visit(ReturnStmtNode<NodeInfo> &node) override;
  void visit(ArgumentNode<NodeInfo> &node) override;

  void enter(BlockNode<NodeInfo> &node) override;
  void exit(BlockNode<NodeInfo> &node) override;
  void enter(MethodDeclNode<NodeInfo> &node) override;
  void exit(MethodDeclNode<NodeInfo> &node) override;
  void enter(ConstructorDeclNode<NodeInfo> &node) override;
  void exit(ConstructorDeclNode<NodeInfo> &node) override;
  void enter(ProgramNode<NodeInfo> &node) override;
  void exit(ProgramNode<NodeInfo> &node) override;

 private:
  int current_stack_offset = 0;
  int max_stack_offset = 0;
  std::vector<int> scope_starts;

  int align16(int n) { return (n + 15) & ~15; }

  int slot_size_for(const VarDeclNode<NodeInfo> &node) {
    // assuming int = 8, string = 16
    if (node.extra.resolved_type == TokenType::TOKEN_STRING) return 16;
    return 8;
  }

  void add_param_symbol(ParamNode<NodeInfo> &node);

  bool types_compatible(const std::string &declared_type_name,
                        const TypeInfo &expr_type_info) {
    TokenType declared_builtin = resolve_type(declared_type_name);
    if (declared_builtin != TokenType::TOKEN_UNKNOWN &&
        declared_builtin != TokenType::TOKEN_DATA_TYPE) {
      return declared_builtin == expr_type_info.token_type;
    }

    Symbol *declared_symbol = lookup_symbol(declared_type_name);
    if (declared_symbol && declared_symbol->is_class) {
      return declared_type_name == expr_type_info.type_name;
    }

    return false;
  }

  TypeInfo check_binary_op(TokenType op, const TypeInfo &left,
                           const TypeInfo &right) {
    switch (op) {
      // arithmetic operators: +, -, *, /
      case TokenType::TOKEN_PLUS:
      case TokenType::TOKEN_MULTIPLY:
        if (left.token_type == TokenType::TOKEN_STRING) {
          return {TokenType::TOKEN_STRING, "string"};
        }
      case TokenType::TOKEN_MINUS:
      case TokenType::TOKEN_DIVIDE:
        if (left.token_type == TokenType::TOKEN_INT &&
            right.token_type == TokenType::TOKEN_INT)
          return {TokenType::TOKEN_INT, "int"};
        // only doing int and int for now, could add string and other
        return {TokenType::TOKEN_UNKNOWN, ""};

      // comparison operators: <, >, <=, >=, ==, !=
      case TokenType::TOKEN_LT:
      case TokenType::TOKEN_GT:
      case TokenType::TOKEN_LEQ:
      case TokenType::TOKEN_GEQ:
        if (left.token_type == TokenType::TOKEN_INT &&
            right.token_type == TokenType::TOKEN_INT) {
          return {TokenType::TOKEN_INT, "int"};
        }
        return {TokenType::TOKEN_UNKNOWN, ""};
      case TokenType::TOKEN_DEQ:
      case TokenType::TOKEN_NEQ:
        if (left.token_type == right.token_type &&
            (left.token_type == TokenType::TOKEN_INT ||
             left.token_type == TokenType::TOKEN_STRING)) {
          return {TokenType::TOKEN_INT, "int"};
        }
        return {TokenType::TOKEN_UNKNOWN, ""};

      default:
        return {TokenType::TOKEN_UNKNOWN, ""};
    }
  }

  TokenType resolve_type(const std::string &name) {
    if (name == "int") return TokenType::TOKEN_INT;
    if (name == "string") return TokenType::TOKEN_STRING;
    if (name == "bool") return TokenType::TOKEN_INT;

    Symbol *symbol = lookup_symbol(name);
    if (symbol && symbol->is_class) {
      return TokenType::TOKEN_DATA_TYPE;
    }

    return TokenType::TOKEN_UNKNOWN;
  }

  std::string get_type_name_from_token(TokenType type) {
    switch (type) {
      case TokenType::TOKEN_INT:
        return "int";
      case TokenType::TOKEN_STRING:
        return "string";
      default:
        return "unknown";
    }
  }
};

#endif  // TYPECHECKER_H_
