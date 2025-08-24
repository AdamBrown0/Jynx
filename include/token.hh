#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

enum class TokenType {
  TOKEN_ID,
  TOKEN_EQUALS,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LBRACKET,
  TOKEN_RBRACKET,
  TOKEN_COLON,
  TOKEN_COMMA,
  TOKEN_LT,
  TOKEN_GT,
  TOKEN_LEQ,
  TOKEN_GEQ,
  TOKEN_ARROW_RIGHT,
  TOKEN_INT,
  TOKEN_STRING,
  TOKEN_STATEMENT,
  TOKEN_SEMICOLON,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_DIVIDE,
  TOKEN_MULTIPLY,
  TOKEN_LSHIFT,
  TOKEN_RSHIFT,
  TOKEN_COMMENT,
  TOKEN_EOF,
};

class Token {
 public:
  Token(std::string value, TokenType type) {
    this->value = value;
    this->type = type;
    position = 0;
  }

 private:
  std::string value;
  TokenType type;
  int position;
};

#endif  // TOKEN_H_
