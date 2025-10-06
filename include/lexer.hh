#ifndef LEXER_HH
#define LEXER_HH

#include <istream>
#include <string>

#include "log.hh"
#include "token.hh"
#include "trie.hh"

class Lexer {
 public:

  /// Set up built in keywords (types, flow control)
  explicit Lexer(std::istream& input) : in(input), location() {
    keywords.insert("int", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("string", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("bool", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("class", TokenType::TOKEN_DATA_TYPE);
    keywords.insert("if", TokenType::KW_IF);
    keywords.insert("else", TokenType::KW_ELSE);
  }

  /// Returns the next token from the source file
  Token next_token();

  int getLine() { return location.line; }
  int getCol() { return location.col; }
  SourceLocation getLocation() { return location; }

 private:
  /// @cond INTERNAL
  std::istream& in;
  SourceLocation location;
  KeywordTrie keywords;
  /// @endcond

  /// Wrapper for making a token with type and value
  Token make_token(TokenType type, std::string value) const {
    return Token(type, value, location);
  }

  /// Advance lexer to next position
  void advance();
  /// Skip whitespace and comments
  void skip_whitespace();
  /// Lex identifier or keyword
  Token identifier();
  /// @return Number token
  Token number();
  /// @return String token
  Token string_literal();
};

#endif  // LEXER_HH
