#ifndef LEXER_HH
#define LEXER_HH

#include <istream>
#include <optional>
#include <string>

#include "token.hh"
#include "trie.hh"

class Lexer {
 public:
  explicit Lexer(std::istream& input) : in(input), line(0), col(0) {
    keywords.insert("int", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("string", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("class", TokenType::TOKEN_DATA_TYPE);
  }

  Token next_token();

  int getLine() { return line; }
  int getCol() { return col; }

 private:
  std::istream& in;
  int line;
  int col;

  KeywordTrie keywords;

  Token make_token(TokenType type, std::string value) const {
    return Token(type, value, line, col);
  }

  void advance();
  void print();
  void skip_whitespace();
  Token identifier();
  Token number();
  Token string_literal();
};

#endif  // LEXER_HH
