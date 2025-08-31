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
  enum class Kind {
    Binary,
    Literal,
    Variable,
    Call,
  };
  Kind kind;
};

struct VarDeclNode : ExprNode {
  TokenType type;
  std::string name;
  ExprNode* initializer;
};

struct BinaryExprNode : ExprNode {
  std::unique_ptr<ExprNode> left;
  TokenType operand;
  std::unique_ptr<ExprNode> right;
};

struct LiteralNode : ExprNode {
  // fuck
};

#endif  // AST_H_
