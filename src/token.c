#include "token.h"

#include <stdlib.h>

/// Initialise a token
/// @param value Token value
/// @param type Token type
/// @return Token
token_t* init_token(char* value, const int type) {
  token_t* token = calloc(1, sizeof(struct TOKEN_STRUCT));
  token->value = value;
  token->type = type;
  return token;
}
