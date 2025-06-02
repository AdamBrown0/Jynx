#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lexer.h"
#include "../include/macros.h"
#include "../include/token.h"

/// Initialise lexer
/// @param src Source for lexer
/// @return Lexer
lexer_t* init_lexer(char* src) {
  lexer_t* lexer = calloc(1, sizeof(struct LEXER_STRUCT));
  lexer->src = src;
  lexer->src_size = strlen(src);
  lexer->index = 0;
  lexer->current = src[lexer->index];
  return lexer;
}

/// Advance to the next index in lexer
/// @param lexer Lexer to advance
void lexer_advance(lexer_t* lexer) {
  if (!(lexer->index < lexer->src_size && lexer->current != '\0')) {
    fprintf(stderr, "Out of source file");
  }
  lexer->index++;
  lexer->current = lexer->src[lexer->index];
}

/// Peek the character at an offset from current position, this doesn't modify the lexer
/// @param lexer Lexer to peek
/// @param offset Offset to peek at
/// @return Character at offset in lexer
char lexer_peek(const lexer_t* lexer, const int offset) { return lexer->src[MIN(lexer->index + offset, lexer->src_size)]; }

/// Peek the next character, this doesn't modify the lexer
/// @param lexer Lexer to peek
/// @return Next character in lexer
char lexer_peek_next(const lexer_t* lexer) { return lexer_peek(lexer, 1); }

/// Advance to the next index in lexer, returning token
/// @param lexer Lexer to peek
/// @param token Token to return
/// @return Token
token_t* lexer_advance_with(lexer_t* lexer, token_t* token) {
  lexer_advance(lexer);
  return token;
}

/// Skip whitespace in lexer
/// @param lexer Lexer
void lexer_skip_whitespace(lexer_t* lexer) {
  while (' ' == lexer->current || '\t' == lexer->current || '\n' == lexer->current || '\r' == lexer->current ||
         '\f' == lexer->current) {
    lexer_advance(lexer);
  }
}

// TODO: move this to util.c
void* safe_realloc(void* value, const size_t size) {
  void* ret = realloc(value, size);
  if (!ret) {
    exit(1);
  }
  return ret;
}

char* strcat_and_realloc(char* dest, const char* src) {
  size_t new_len = strlen(dest) + strlen(src) + 1;
  dest = safe_realloc(dest, new_len);
  strcat(dest, src);
  return dest;
}

/// Parse identifiers in lexer
/// @param lexer Lexer to parse
/// @return Identifier token
token_t* lexer_parse_id(lexer_t* lexer) {
  char* value = calloc(1, sizeof(char));
  while (isalpha(lexer->current) || lexer->current == '_') {
    value = safe_realloc(value, (strlen(value) + 2) * sizeof(char));
    strcat(value, (char[]){lexer->current, 0});
    lexer_advance(lexer);
  }
  return init_token(value, TOKEN_ID);
}

/// Parse integers in lexer
/// @param lexer Lexer to parse
/// @return Integer token
token_t* lexer_parse_int(lexer_t* lexer) {
  char* value = calloc(1, sizeof(char));
  while (isdigit(lexer->current)) {
    value = safe_realloc(value, (strlen(value) + 2) * sizeof(char));
    strcat(value, (char[]){lexer->current, 0});
    lexer_advance(lexer);
  }

  return init_token(value, TOKEN_INT);
}

/// Parse strings in lexer
/// @param lexer Lexer to parse
/// @return String token
token_t* lexer_parse_string(lexer_t* lexer) {
  lexer_advance(lexer); // skip first quote marks

  char* value = calloc(1, sizeof(char));

  char c;
  while ((c = lexer->current) != '"' && c != '\0') {
    // check for escape sequences
    if (c == '\\') {
      lexer_advance(lexer);
      switch (c) {
        case 'n': value = strcat_and_realloc(value, "\n"); break;
        case 'r': value = strcat_and_realloc(value, "\r"); break;
        case 't': value = strcat_and_realloc(value, "\t"); break;
        case '"': value = strcat_and_realloc(value, "\""); break;
        case '\\': value = strcat_and_realloc(value, "\\"); break;
        default: value = strcat_and_realloc(value, (char[]){c, 0}); break;
      }
    } else { // regular character
      value = strcat_and_realloc(value, (char[]){c, 0});
    }
    lexer_advance(lexer);
  }

  if (c != '"') { // probably replace with proper error handler
    fprintf(stderr, "Unterminated string literal\n");
    exit(1);
  }

  return init_token(value, TOKEN_STRING);
}

/// Lex next token
/// @param lexer Lexer
/// @return Next token, type TOKEN_EOF at end of lexer->src
token_t* lexer_next_token(lexer_t* lexer) {
  lexer_skip_whitespace(lexer);

  char c;
  switch ((c = lexer->current)) {
    case '=': return lexer_advance_with(lexer, init_token("=", TOKEN_EQUALS));
    case '(': return init_token("(", TOKEN_LPAREN);
    case ')': return init_token(")", TOKEN_RPAREN);
    case '{': return init_token("{", TOKEN_LBRACE);
    case '}': return init_token("}", TOKEN_RBRACE);
    case '[': return init_token("[", TOKEN_LBRACKET);
    case ']': return init_token("]", TOKEN_RBRACKET);
    case ':': return init_token(":", TOKEN_COLON);
    case ',': return init_token(",", TOKEN_COMMA);
    case '<': {
      if (lexer_peek_next(lexer) == '=') return init_token("<=", TOKEN_LEQ);
      if (lexer_peek_next(lexer) == '<') return init_token("<<", TOKEN_LSHIFT);
      return init_token("<", TOKEN_LT);
    }
    case '>': {
      if (lexer_peek_next(lexer) == '=') return init_token(">=", TOKEN_GEQ);
      if (lexer_peek_next(lexer) == '>') return init_token(">>", TOKEN_RSHIFT);
      return init_token(">", TOKEN_GT);
    }
    case '-': {
      if (lexer_peek_next(lexer) == '>') return init_token("->", TOKEN_ARROW_RIGHT);
      return init_token("-", TOKEN_MINUS);
    }
    case ';': return lexer_advance_with(lexer, init_token(";", TOKEN_SEMICOLON));
    case '+': return init_token("+", TOKEN_PLUS);
    case '*': return init_token("*", TOKEN_MULTIPLY);
    case '/': {
      if (lexer_peek_next(lexer) == '/') return init_token("//", TOKEN_COMMENT);
      return init_token("/", TOKEN_DIVIDE);
    }
    case '"': return lexer_parse_string(lexer);
    default: {
      if (isalpha(c) || c == '_') return lexer_parse_id(lexer);
      if (isdigit(c)) return lexer_parse_int(lexer);
    }
  }

  return init_token(0, TOKEN_EOF);
}
