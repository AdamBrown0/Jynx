#include "../include/lexer.hh"

#include "../include/token.hh"

Token Lexer::next_token() {
  return Token(std::string("Test"), TokenType::TOKEN_STRING);
}
