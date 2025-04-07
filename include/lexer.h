#ifndef LEXER_H
#define LEXER_H
#include <stddef.h>

typedef struct LEXER_STRUCT {
  char* src;
  size_t src_size;
  char current;
  unsigned int index;
} lexer_t;

lexer_t* init_lexer(char* src);

#endif // LEXER_H
