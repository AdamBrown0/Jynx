#ifndef LEXER_HH
#define LEXER_HH

#include <istream>
#include <string>

#include "log.hh"
#include "token.hh"
#include "trie.hh"
#include "visitor/visitor.hh"

class Lexer {
 public:
  /// Set up built in keywords (types, flow control)
  explicit Lexer(std::istream& input, CompilerContext& ctx)
      : in(input), location(), context(ctx) {
    context.keywords.insert("int", TokenType::TOKEN_DATA_TYPE);
    context.keywords.insert("string", TokenType::TOKEN_DATA_TYPE);
    context.keywords.insert("char", TokenType::TOKEN_DATA_TYPE);
    context.keywords.insert("bool", TokenType::TOKEN_DATA_TYPE);
    context.keywords.insert("void", TokenType::TOKEN_DATA_TYPE);
    context.keywords.insert("public", TokenType::KW_ACCESS_MODIFIER);
    context.keywords.insert("private", TokenType::KW_ACCESS_MODIFIER);
    context.keywords.insert("protected", TokenType::KW_ACCESS_MODIFIER);
    context.keywords.insert("class", TokenType::KW_CLASS);
    context.keywords.insert("if", TokenType::KW_IF);
    context.keywords.insert("else", TokenType::KW_ELSE);
    context.keywords.insert("return", TokenType::KW_RETURN);
    context.keywords.insert("while", TokenType::KW_WHILE);
    context.keywords.insert("constructor", TokenType::KW_CONSTRUCTOR);
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
  CompilerContext& context;
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
  Token char_literal();
};

#endif  // LEXER_HH
