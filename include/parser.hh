#ifndef PARSER_H_
#define PARSER_H_

#include <deque>

#include "ast.hh"
#include "lexer.hh"
#include "token.hh"

class Parser {
 public:
  explicit Parser(Lexer& lexer) : lexer(lexer), current(Token()) {}

  ProgramNode<ParseExtra>* parseProgram();

 private:
  /// @internal
  Lexer& lexer;
  /// @internal
  Token current;

  /// @internal
  std::deque<Token> peeked_tokens;

  /// Returns the binary precedence for operator
  /// Used in binary expressions (i.e. 5 + 2 * 3)
  /// @return Precedence
  int getBinaryPrecedence(TokenType type) {
    using Tk = TokenType;
    switch (type) {
      case Tk::TOKEN_MULTIPLY:
      case Tk::TOKEN_DIVIDE:
        return 4;
      case Tk::TOKEN_PLUS:
      case Tk::TOKEN_MINUS:
        return 3;
      case Tk::TOKEN_EQUALS:
        return 2;
      case Tk::TOKEN_DEQ:
      case Tk::TOKEN_GT:
      case Tk::TOKEN_GEQ:
      case Tk::TOKEN_LT:
      case Tk::TOKEN_LEQ:
        return 1;
      default:
        return -1;
    }
  }

  /// Returns the unary precedence for operator
  /// Used in unary expressions (i.e. -1)
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

  /// Peek at the next token in the stream
  /// @return Next token
  Token peek(size_t count) {
    while (peeked_tokens.size() < count)
      peeked_tokens.push_back(lexer.next_token());

    return peeked_tokens.at(count - 1);
  }

  /// Advance parser to next token in token stream
  /// @return Next token
  Token advance() {
    if (!peeked_tokens.empty()) {
      current = peeked_tokens.front();
      peeked_tokens.pop_front();
    } else {
      current = lexer.next_token();
    }
    return current;
  }

  /// Return the current token, then advance to next token
  /// @return Current token
  Token ret_advance() {
    Token ret = current;
    advance();
    return ret;
  }

  /// @cond INTERNAL
  using _StmtNode = StmtNode<ParseExtra>;
  using _ExprNode = ExprNode<ParseExtra>;
  using _ASTNode = ASTNode<ParseExtra>;
  /// @endcond

  // statements
  _StmtNode* parseStatement();
  _StmtNode* parseBlock();
  _StmtNode* parseVarDecl();
  _StmtNode* parseIfStmt();
  // _StmtNode* parseElseStmt();
  _StmtNode* parseReturnStmt();
  _StmtNode* parseWhileStmt();
  _StmtNode* parseExprStmt();

  // class related
  _StmtNode* parseClass();
  _StmtNode* parseFieldDecl();
  _StmtNode* parseMethodDecl();
  _StmtNode* parseConstructorDecl();

  // expressions
  _ExprNode* parseExpr();
  _ExprNode* parseBinaryExpr();
  _ExprNode* parseBinaryExpr(int);
  _ExprNode* parseUnaryExpr();
  _ExprNode* parseLiteralExpr();
  _ExprNode* parseIdentifierExpr();
  // _StmtNode* parseAssignmentExpr(); // fairly sure ts isnt needed
  _ExprNode* parseMethodCall();

  // supporting
  _ASTNode* parseArgument();
  _ASTNode* parseParam();
};

#endif  // PARSER_H_
