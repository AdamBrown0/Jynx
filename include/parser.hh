#ifndef PARSER_H_
#define PARSER_H_

#include "ast.hh"
#include "lexer.hh"
#include "token.hh"

class Parser {
 public:
  explicit Parser(Lexer& lexer) : lexer(lexer), current(Token()) {}

  ProgramNode<ParseExtra>* parseProgram();

 private:
  Lexer& lexer;
  Token current;

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
    using Tk = TokenType;
    switch (type) {
      case Tk::TOKEN_PLUS:
      case Tk::TOKEN_MINUS:
        return 3;  // Higher precedence than binary operators
      default:
        return -1;
    }
  }

  Token advance() {
    current = lexer.next_token();
    return current;
  }

  Token ret_advance() {
    Token ret = current;
    advance();
    return ret;
  }

  using _StmtNode = StmtNode<ParseExtra>;
  // statements
  _StmtNode* parseStatement();
  _StmtNode* parseBlock();
  _StmtNode* parseVarDecl();
  _StmtNode* parseIfStmt();
  _StmtNode* parseElseStmt();
  _StmtNode* parseReturnStmt();
  _StmtNode* parseExprStmt();

  // class related
  _StmtNode* parseClass();
  _StmtNode* parseFieldDecl();
  _StmtNode* parseMethodDecl();
  _StmtNode* parseConstructorDecl();

  using _ExprNode = ExprNode<ParseExtra>;
  // expressions
  _ExprNode* parseExpr();
  _ExprNode* parseBinaryExpr();
  _ExprNode* parseBinaryExpr(int);
  _ExprNode* parseUnaryExpr();
  _ExprNode* parseLiteralExpr();
  _ExprNode* parseIdentifierExpr();
  _ExprNode* parseAssignmentExtr();
  _ExprNode* parseMethodCall();

  using _ASTNode = ASTNode<ParseExtra>;
  // supporting
  _ASTNode* parseArgument();
  _ASTNode* parseParam();
};

#endif  // PARSER_H_
