#ifndef PARSER_H_
#define PARSER_H_

#include "ast.hh"
#include "lexer.hh"
#include "token.hh"

class Parser {
 public:
  explicit Parser(Lexer& lexer) : lexer(lexer), current(Token()) {}

  ASTNode* parseProgram();

 private:
  Lexer& lexer;
  Token current;

  bool check(TokenType type);

    int getBinaryPrecedence(TokenType type) {
      using Tk = TokenType;
      switch (type) {
        case Tk::TOKEN_MULTIPLY:
        case Tk::TOKEN_DIVIDE:
          return 2;
        case Tk::TOKEN_PLUS:
        case Tk::TOKEN_MINUS:
          return 1;
        default:
          return -1;
      }
    }

    int getUnaryPrecedence(TokenType type) {
      switch (type) {
        default: return -1;
      }
    }

  // bool match
  ExprNode* parseVarDecl();
  ExprNode* parseExpr();
  ExprNode* parseBinaryExpr();
  ExprNode* parseBinaryExpr(int);
  ExprNode* parseLiteralExpr();
};

#endif  // PARSER_H_
