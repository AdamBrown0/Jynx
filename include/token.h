#ifndef TOKEN_H
#define TOKEN_H

typedef struct TOKEN_STRUCT {
  char* value;
  enum {
    TOKEN_ID,
    TOKEN_EQUALS,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LEQ,
    TOKEN_GEQ,
    TOKEN_ARROW_RIGHT,
    TOKEN_INT,
    TOKEN_STRING,
    TOKEN_STATEMENT,
    TOKEN_SEMICOLON,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_DIVIDE,
    TOKEN_MULTIPLY,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,
    TOKEN_COMMENT,
    TOKEN_EOF,
  } type;
  int position;
} token_t;

token_t* init_token(char* value, int type);

#endif // TOKEN_H
