#ifndef LEXER_HH
#define LEXER_HH

#include <cstring>
#include <istream>
#include <optional>
#include <string>

#include "token.hh"

class Lexer {
 public:
  explicit Lexer(std::istream& input) : in(input), line(1) {
    keywords.insert("int", TokenType::KW_INT);
    keywords.insert("string", TokenType::KW_STRING);
    keywords.insert("class", TokenType::KW_CLASS);
  }

  std::optional<Token> next_token();

 private:
  std::istream& in;
  int line;

  KeywordTrie keywords;

  Token make_token(TokenType type, std::string value) {
    return Token(type, value, line);
  }

  void advance();
  void print();
  // char peek();
  // char peek_next();
  // Token advance_with();
  void skip_whitespace();
  Token identifier();
  Token number();
  Token string_literal();
};

#endif  // LEXER_HH
