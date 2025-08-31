#ifndef PARSER_H_
#define PARSER_H_

#include "ast.hh"
#include "lexer.hh"
#include "token.hh"

class Parser {
 public:
  explicit Parser(Lexer& lexer) : lexer(lexer), current(Token(TokenType::TOKEN_ARROW_RIGHT, "bob", 10)) {}

  ASTNode* parseProgram();

 private:
  Lexer& lexer;
  Token current;

  bool check(TokenType type);
  // bool match
  VarDeclNode* parseVarDecl();
  ExprNode* parseExpr();
  BinaryExprNode* parseBinaryExpr();
};

#endif  // PARSER_H_
