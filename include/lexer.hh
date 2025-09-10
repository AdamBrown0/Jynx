#ifndef LEXER_HH
#define LEXER_HH

#include <istream>
#include <string>

#include "log.hh"
#include "token.hh"
#include "trie.hh"

class Lexer {
 public:
  explicit Lexer(std::istream& input) : in(input), location() {
    keywords.insert("int", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("string", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("bool", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("class", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("if", TokenType::KW_IF);
  }

  Token next_token();

  int getLine() { return location.line; }
  int getCol() { return location.col; }
  SourceLocation getLocation() { return location; }

 private:
  std::istream& in;
  SourceLocation location;

  KeywordTrie keywords;

  Token make_token(TokenType type, std::string value) const {
    return Token(type, value, location);
  }

  void advance();
  void print();
  void skip_whitespace();
  Token identifier();
  Token number();
  Token string_literal();
};

#endif  // LEXER_HH
