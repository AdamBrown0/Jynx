#ifndef TYPECHECKER_H_
#define TYPECHECKER_H_

#include <unordered_map>

#include "ast.hh"
#include "visitor.hh"

class TypeCheckerVisitor : public ASTVisitor<ParseExtra> {
 public:
  TypeCheckerVisitor() {}

  TypeCheckerVisitor(const std::unordered_map<std::string, Symbol> &symbols) {
    global_symbols = symbols;
  }

  const std::unordered_map<ASTNode<ParseExtra> *, TokenType> get_expr_types()
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
  void visit(ReturnStmtNode<ParseExtra> &node) override;
  void visit(ClassNode<ParseExtra> &node) override;
  void visit(FieldDeclNode<ParseExtra> &node) override;
  void visit(MethodDeclNode<ParseExtra> &node) override;
  void visit(ConstructorDeclNode<ParseExtra> &node) override;

 private:
  std::unordered_map<ASTNode<ParseExtra> *, TokenType> expr_types;

  TokenType get_expr_type(ASTNode<ParseExtra> *node) {
    if (!node) return TokenType::TOKEN_UNKNOWN;

    auto it = expr_types.find(node);
    if (it != expr_types.end()) return it->second;

    return TokenType::TOKEN_UNKNOWN;
  }

  void set_expr_type(ASTNode<ParseExtra> *node, TokenType type) {
    if (node) expr_types[node] = type;
  }

  bool types_compatible(TokenType left, TokenType right) {
    return left == right;
  }

  TokenType check_binary_op(TokenType op, TokenType left, TokenType right) {
    switch (op) {
      // arithmetic operators: +, -, *, /
      case TokenType::TOKEN_PLUS:
      case TokenType::TOKEN_MINUS:
      case TokenType::TOKEN_MULTIPLY:
      case TokenType::TOKEN_DIVIDE:
        if (left == TokenType::TOKEN_INT && right == TokenType::TOKEN_INT)
          return TokenType::TOKEN_INT;
        if (left == TokenType::TOKEN_STRING) // "str" + 2 is valid, 2 + "str" is not
          return TokenType::TOKEN_STRING;
        // only doing int and int for now, could add string and other
        return TokenType::TOKEN_UNKNOWN;

        // comparison stuff

      default:
        return TokenType::TOKEN_UNKNOWN;
    }
  }
};

#endif  // TYPECHECKER_H_
