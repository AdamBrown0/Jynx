#ifndef LEXER_HH
#define LEXER_HH

#include <cstring>
#include <string>

#include "token.hh"

class Lexer {
 public:
  Lexer(std::string src) {
    this->src = src;
    src_size = std::strlen(src.c_str());
    index = 0;
    current = src[index];
  }

  Token next_token();

 private:
  std::string src;
  size_t src_size;
  char current;
  unsigned int index;

  void advance();
  void print();
  char peek();
  char peek_next();
  Token advance_with();
  void skip_whitespace();
  Token parse_id();
  Token parse_int();
  Token parse_string();
};

#endif  // LEXER_HH
