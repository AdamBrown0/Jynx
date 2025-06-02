#ifndef LEXER_H
#define LEXER_H
#include <stddef.h>

#include "token.h"

typedef struct LEXER_STRUCT {
  char* src;
  size_t src_size;
  char current;
  unsigned int index;
} lexer_t;

lexer_t* init_lexer(char* src);

token_t* lexer_next_token(lexer_t* lexer);

#endif // LEXER_H
