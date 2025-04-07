#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "macros.h"
#include "token.h"

lexer_t* init_lexer(char* src) {
  lexer_t* lexer = calloc(1, sizeof(struct LEXER_STRUCT));
  lexer->src = src;
  lexer->src_size = strlen(src);
  lexer->index = 0;
  lexer->current = src[lexer->index];
  return lexer;
}

void lexer_advance(lexer_t* lexer) {
  if (!(lexer->index < lexer->src_size && lexer->current != '\0')) {
    fprintf(stderr, "Out of source file");
  }
  lexer->index++;
  lexer->current = lexer->src[lexer->index];
}

char lexer_peek(const lexer_t* lexer, const int offset) { return lexer->src[MIN(lexer->index + offset, lexer->src_size)]; }
char lexer_peek_next(const lexer_t* lexer) { return lexer_peek(lexer, 1); }

token_t* lexer_advance_with(lexer_t* lexer, token_t* token) {
  lexer_advance(lexer);
  return token;
}

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

token_t* lexer_parse_id(lexer_t* lexer) {
  char* value = calloc(1, sizeof(char));
  while (isalpha(lexer->current)) {
    value = safe_realloc(value, (strlen(value) + 2) * sizeof(char));
    strcat(value, (char[]){lexer->current, 0});
    lexer_advance(lexer);
  }
  return init_token(value, TOKEN_ID);
}

token_t* lexer_parse_int(lexer_t* lexer) {

  char* value = calloc(1, sizeof(char));
  while (isdigit(lexer->current)) {
    value = safe_realloc(value, (strlen(value) + 2) * sizeof(char));
    strcat(value, (char[]){lexer->current, 0});
    lexer_advance(lexer);
  }

  return init_token(value, TOKEN_INT);
}

token_t* lexer_next_token(lexer_t* lexer) {
  while (lexer->current != EOF) {
    if (isalnum(lexer->current))
      return lexer_advance_with(lexer, lexer_parse_id(lexer));
  }

  char c;
  switch ((c = lexer->current)) {
    case '=': return init_token("=", TOKEN_EQUALS);
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
    case isdigit(c): return lexer_parse_int(lexer);
    // case '': return init_token("", TOKEN_);
    // case '': return init_token("", TOKEN_);
    default: break;
  }

  return init_token(0, TOKEN_EOF);
}
