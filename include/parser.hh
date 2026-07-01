#ifndef PARSER_H_
#define PARSER_H_

#include <deque>
#include <optional>

#include "ast.hh"
#include "lexer.hh"
#include "token.hh"

class Parser {
 public:
  explicit Parser(Lexer& lexer, CompilerContext& context)
      : lexer(lexer), current(Token()), ctx(context) {}

  ProgramNode* parseProgram();

 private:
  /// @internal
  Lexer& lexer;
  /// @internal
  Token current;

  CompilerContext& ctx;

  /// @internal
  std::deque<Token> peeked_tokens;

  /// Returns the binary precedence for operator
  /// Used in binary expressions (i.e. 5 + 2 * 3)
  /// @return Precedence
  int getBinaryPrecedence(TokenType type) {
    using Tk = TokenType;
    switch (type) {
      case Tk::TOKEN_DATA_TYPE:
        return 5;
      case Tk::TOKEN_MULTIPLY:
      case Tk::TOKEN_DIVIDE:
        return 4;
      case Tk::TOKEN_PLUS:
      case Tk::TOKEN_MINUS:
        return 3;
      case Tk::TOKEN_EQUALS:
        return 2;
      case Tk::TOKEN_DEQ:
      case Tk::TOKEN_NEQ:
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
        return 100;  // Higher precedence than binary operators
      default:
        return -1;
    }
  }

  /// Returns true if current token type matches the one passed
  inline bool match(TokenType type) const { return current.getType() == type; }

  inline bool match(const Type* type) {
    return current.getValue() == type->to_string();
  }

  const Type* get_current_type() {
    if (current.getType() == TokenType::TOKEN_INT) return ctx.get_int32_type();
    if (current.getType() == TokenType::TOKEN_BOOL) return ctx.get_bool_type();
    if (current.getType() == TokenType::TOKEN_STRING)
      return ctx.make_pointer_type(ctx.get_char_type());
    if (current.getType() == TokenType::TOKEN_CHAR) return ctx.get_char_type();
    return ctx.get_void_type();
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

  // statements
  StmtNode* parseStatement();
  StmtNode* parseBlock();
  StmtNode* parseIfStmt();
  // StmtNode* parseElseStmt();
  StmtNode* parseReturnStmt();
  StmtNode* parseWhileStmt();
  StmtNode* parseExprStmt();

  // class related
  StmtNode* parseClass();
  ClassMemberNode* parseClassMember();
  ClassMemberNode* parseFieldDecl(std::optional<Token>);
  ClassMemberNode* parseMethodDecl(std::optional<Token>);
  ClassMemberNode* parseConstructorDecl();

  // expressions
  ExprNode* parseExpr();
  ExprNode* parseBinaryExpr();
  ExprNode* parseBinaryExpr(int);
  ExprNode* parseUnaryExpr();
  ExprNode* parseLiteralExpr();
  ExprNode* parseIdentifierExpr();
  // StmtNode* parseAssignmentExpr(); // fairly sure ts isnt needed
  ExprNode* parseMethodCall();
  ExprNode* parseVarDecl();

  // supporting
  ASTNode* parseArgument();
  ASTNode* parseParam();
  const Type* parseType();
};

#endif  // PARSER_H_
