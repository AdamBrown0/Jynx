#ifndef AST_H_
#define AST_H_

#include <memory>
#include <string>

#include "token.hh"

struct ASTNode {
  int line;
  int col;

  virtual ~ASTNode() = default;
};

struct ExprNode : ASTNode {
  ExprNode() {}
};

struct VarDeclNode : ExprNode {
  Token type_token;
  Token identifier;
  std::unique_ptr<ExprNode> initializer;

  VarDeclNode(Token type_token, Token identifier, ExprNode* initializer) : type_token(std::move(type_token)) {
    this->identifier = identifier;
    this->initializer = std::unique_ptr<ExprNode>(initializer);
  }
};

struct BinaryExprNode : ExprNode {
  std::unique_ptr<ExprNode> left;
  Token op;
  std::unique_ptr<ExprNode> right;

  BinaryExprNode(ExprNode* left, Token op, ExprNode* right) {
    this->left = std::unique_ptr<ExprNode>(left);
    this->op = op;
    this->right = std::unique_ptr<ExprNode>(right);
  }
};

struct UnaryExprNode : ExprNode {
  Token op;
  Token operand;

  UnaryExprNode(Token op, Token operand) : op(std::move(op)), operand(std::move(operand)) {}
};

struct LiteralExprNode : ExprNode {
  Token literal_token;

  LiteralExprNode(Token token, int line, int col) : literal_token(token) {
    this->literal_token = token;
    this->line = line;
    this->col = col;
  }
};

#endif  // AST_H_
