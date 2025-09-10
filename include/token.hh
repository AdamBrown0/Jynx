#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>
#include "sourcelocation.hh"

#define TOKEN_LIST     \
  X(TOKEN_ID)          \
  X(TOKEN_EQUALS)      \
  X(TOKEN_LPAREN)      \
  X(TOKEN_RPAREN)      \
  X(TOKEN_LBRACE)      \
  X(TOKEN_RBRACE)      \
  X(TOKEN_LBRACKET)    \
  X(TOKEN_RBRACKET)    \
  X(TOKEN_COLON)       \
  X(TOKEN_COMMA)       \
  X(TOKEN_LT)          \
  X(TOKEN_GT)          \
  X(TOKEN_LEQ)         \
  X(TOKEN_GEQ)         \
  X(TOKEN_ARROW_RIGHT) \
  X(TOKEN_INT)         \
  X(TOKEN_STRING)      \
  X(TOKEN_STATEMENT)   \
  X(TOKEN_SEMICOLON)   \
  X(TOKEN_PLUS)        \
  X(TOKEN_MINUS)       \
  X(TOKEN_DIVIDE)      \
  X(TOKEN_MULTIPLY)    \
  X(TOKEN_LSHIFT)      \
  X(TOKEN_RSHIFT)      \
  X(TOKEN_COMMENT)     \
  X(TOKEN_DATA_TYPE)   \
  X(KW_IF)             \
  X(TOKEN_UNKNOWN)     \
  X(TOKEN_EOF)

enum class TokenType {
#define X(name) name,
  TOKEN_LIST
#undef X
};

class Token {
 public:
  Token() {}
  Token(TokenType type, std::string value, SourceLocation loc)
      : value(value), type(type), location(loc) {}

  TokenType getType() const { return type; }
  std::string getValue() const { return value; }
  int getLine() const { return location.line; }
  int getCol() const { return location.col; }

  void print(); // Declaration only - implementation in token.cc

  constexpr const char* to_string() {
    switch (type) {
#define X(name)         \
  case TokenType::name: \
    return #name;
      TOKEN_LIST
#undef X
      default:
        return "UNKNOWN";
    }
  }

 private:
  std::string value;
  TokenType type;
  SourceLocation location;
};

#endif  // TOKEN_H_
