#include "../include/lexer.hh"

#include <cctype>
#include <optional>

#include "../include/token.hh"

std::optional<Token> Lexer::next_token() {
  skip_whitespace();

  if (in.eof()) return make_token(TokenType::TOKEN_EOF, "");

  char c = in.peek();

  // identifiers
  if (std::isalpha(c) || c == '_') return identifier();

  // numbers
  if (std::isdigit(c)) return number();

  // strings
  if (c == '"') return string_literal();

  in.get();
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
      char c = in.get();
      if (c == '=') {
        return make_token(TokenType::TOKEN_LEQ, "<=");
      }
      if (c == '<') {
        return make_token(TokenType::TOKEN_LSHIFT, "<<");
      }
      in.unget();
      return make_token(TokenType::TOKEN_LT, "<");
    }
    case '>': {
      char c = in.get();
      if (c == '=') {
        return make_token(TokenType::TOKEN_GEQ, ">=");
      }
      if (c == '>') {
        return make_token(TokenType::TOKEN_RSHIFT, ">>");
      }
      in.unget();
      return make_token(TokenType::TOKEN_GT, ">");
    }
    case '-': {
      if (in.peek() == '>') {
        in.get();
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

  return std::nullopt;
}

void Lexer::skip_whitespace() {
  while (!in.eof()) {
    char c = in.peek();
    if (std::isspace(c)) {
      if (c == '\n') line++;
      in.get();
    } else if (c == '/') {
      in.get();                // consume the '/'
      if (in.peek() == '/') {  // line comment
        in.get();              // consume the second '/'
        while (!in.eof() && in.get() != '\n');
        line++;
      } else {
        in.unget();  // put back the '/' - it's a divide operator
        break;
      }
    } else
      break;
  }
}

Token Lexer::identifier() {
  std::string buf;
  while (!in.eof() && (std::isalnum(in.peek()) || in.peek() == '_'))
    buf.push_back(in.get());
  if (TokenType* type = keywords.find(buf)) return make_token(*type, buf);
  return make_token(TokenType::TOKEN_ID, buf);
}

Token Lexer::number() const {
  std::string buf;
  while (!in.eof() && std::isdigit(in.peek())) buf.push_back(in.get());
  return make_token(TokenType::TOKEN_INT, buf);
}

Token Lexer::string_literal() {
  std::string buf;
  in.get();  // skip first quote marks

  while (!in.eof() && in.peek() != '"') {
    char c = in.get();
    if (c == '\n') line++;
    buf.push_back(c);
  }
  in.get();  // skip last quote marks
  return make_token(TokenType::TOKEN_STRING, buf);
}
