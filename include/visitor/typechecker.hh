#ifndef TYPECHECKER_H_
#define TYPECHECKER_H_

#include <unordered_map>

#include "ast.hh"
#include "visitor.hh"

typedef struct TypeInfo {
  TokenType token_type;
  std::string type_name;
} TypeInfo;

class TypeCheckerVisitor : public ASTVisitor<ParseExtra> {
 public:
  TypeCheckerVisitor() {}

  TypeCheckerVisitor(const std::unordered_map<std::string, Symbol> &symbols) {
    global_symbols = symbols;
  }

  const std::unordered_map<ASTNode<ParseExtra> *, TypeInfo> get_expr_types()
      const {
    return expr_types;
  }

  void visit(ASTNode<ParseExtra> &node) override;
  void visit(BinaryExprNode<ParseExtra> &node) override;
  void visit(UnaryExprNode<ParseExtra> &node) override;
  void visit(LiteralExprNode<ParseExtra> &node) override;
  void visit(IdentifierExprNode<ParseExtra> &node) override;
  void visit(AssignmentExprNode<ParseExtra> &node) override;
  void visit(MethodCallNode<ParseExtra> &node) override;
  void visit(ArgumentNode<ParseExtra> &node) override;
  void visit(ParamNode<ParseExtra> &node) override;
  void visit(ProgramNode<ParseExtra> &node) override;
  void visit(BlockNode<ParseExtra> &node) override;
  void visit(VarDeclNode<ParseExtra> &node) override;
  void visit(IfStmtNode<ParseExtra> &node) override;
  void visit(WhileStmtNode<ParseExtra> &) override {};
  void visit(ReturnStmtNode<ParseExtra> &node) override;
  void visit(ClassNode<ParseExtra> &node) override;
  void visit(FieldDeclNode<ParseExtra> &node) override;
  void visit(MethodDeclNode<ParseExtra> &node) override;
  void visit(ConstructorDeclNode<ParseExtra> &node) override;
  void visit(ExprStmtNode<ParseExtra> &node) override;

 private:
  std::unordered_map<ASTNode<ParseExtra> *, TypeInfo> expr_types;

  TypeInfo get_expr_type(ASTNode<ParseExtra> *node) {
    if (!node) return {TokenType::TOKEN_UNKNOWN, ""};
    auto it = expr_types.find(node);
    return (it != expr_types.end()) ? it->second
                                    : TypeInfo{TokenType::TOKEN_UNKNOWN, ""};
  }

  void set_expr_type(ASTNode<ParseExtra> *node, TokenType type,
                     const std::string &type_name = "") {
    if (node) expr_types[node] = {type, type_name};
  }

  void set_expr_type(ASTNode<ParseExtra> *node, TypeInfo type_info) {
    if (node) expr_types[node] = type_info;
  }

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

        // comparison stuff

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
