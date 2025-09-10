#include "../include/lexer.hh"

#include <cctype>
#include <optional>

#include "../include/token.hh"

Token Lexer::next_token() {
  skip_whitespace();

  if (in.eof()) return make_token(TokenType::TOKEN_EOF, "");

  char c = in.peek();

  // identifiers
  if (std::isalpha(c) || c == '_') return identifier();

  // numbers
  if (std::isdigit(c)) return number();

  // strings
  if (c == '"') return string_literal();

  advance();  // consume the character
  switch (c) {
    case '(':
      return make_token(TokenType::TOKEN_LPAREN, "(");
    case ')':
      return make_token(TokenType::TOKEN_RPAREN, ")");
    case '{':
      return make_token(TokenType::TOKEN_LBRACE, "{");
    case '}':
      return make_token(TokenType::TOKEN_RBRACE, "}");
    case '[':
      return make_token(TokenType::TOKEN_LBRACKET, "[");
    case ']':
      return make_token(TokenType::TOKEN_RBRACKET, "]");
    case ':':
      return make_token(TokenType::TOKEN_COLON, ":");
    case ',':
      return make_token(TokenType::TOKEN_COMMA, ",");
    case '<': {
      char next = in.peek();
      if (next == '=') {
        advance();  // consume the '='
        return make_token(TokenType::TOKEN_LEQ, "<=");
      }
      if (next == '<') {
        advance();  // consume the second '<'
        return make_token(TokenType::TOKEN_LSHIFT, "<<");
      }
      return make_token(TokenType::TOKEN_LT, "<");
    }
    case '>': {
      char next = in.peek();
      if (next == '=') {
        advance();  // consume the '='
        return make_token(TokenType::TOKEN_GEQ, ">=");
      }
      if (next == '>') {
        advance();  // consume the second '>'
        return make_token(TokenType::TOKEN_RSHIFT, ">>");
      }
      return make_token(TokenType::TOKEN_GT, ">");
    }
    case '-': {
      if (in.peek() == '>') {
        advance();  // consume the '>'
        return make_token(TokenType::TOKEN_ARROW_RIGHT, "->");
      }
      return make_token(TokenType::TOKEN_MINUS, "-");
    }
    case ';':
      return make_token(TokenType::TOKEN_SEMICOLON, ";");
    case '=':
      return make_token(TokenType::TOKEN_EQUALS, "=");
    case '+':
      return make_token(TokenType::TOKEN_PLUS, "+");
    case '*':
      return make_token(TokenType::TOKEN_MULTIPLY, "*");
    case '/':
      return make_token(TokenType::TOKEN_DIVIDE, "/");
  }

  return make_token(TokenType::TOKEN_UNKNOWN, "");
}

void Lexer::advance() {
  char c = in.get();
  if (c == '\n') {
    location.line++;
    location.col = 0;
  } else {
    location.col++;
  }
}

void Lexer::skip_whitespace() {
  while (!in.eof()) {
    char c = in.peek();
    if (std::isspace(c)) {
      advance();  // This will handle line/col tracking
    } else if (c == '/') {
      advance();                // consume the '/'
      if (in.peek() == '/') {  // line comment
        advance();              // consume the second '/'
        while (!in.eof() && in.peek() != '\n') {
          advance();
        }
        if (!in.eof()) advance();  // consume the newline
      } else {
        in.unget();  // put back the '/' - it's a divide operator
        location.col--;       // adjust column since we put the character back
        break;
      }
    } else
      break;
  }
}

Token Lexer::identifier() {
  std::string buf;
  while (!in.eof() && (std::isalnum(in.peek()) || in.peek() == '_')) {
    buf.push_back(in.peek());
    advance();
  }
  if (TokenType* type = keywords.find(buf)) return make_token(*type, buf);
  return make_token(TokenType::TOKEN_ID, buf);
}

Token Lexer::number() {
  std::string buf;
  while (!in.eof() && std::isdigit(in.peek())) {
    buf.push_back(in.peek());
    advance();
  }
  return make_token(TokenType::TOKEN_INT, buf);
}

Token Lexer::string_literal() {
  std::string buf;
  advance();  // skip first quote marks

  while (!in.eof() && in.peek() != '"') {
    char c = in.peek();
    buf.push_back(c);
    advance();
  }
  advance();  // skip last quote marks
  return make_token(TokenType::TOKEN_STRING, buf);
}
