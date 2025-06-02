#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/token.h"
#include "../include/lexer.h"

// TODO: make this not exit on fail, or add more logging idk
void assert_token(token_t* tok, const char* expected_val, int type) {
  if (!tok) {
    fprintf(stderr, "Expected token, got NULL\n");
    exit(1);
  }

  if (strcmp(tok->value, expected_val) != 0 || tok->type != type) {
    fprintf(stderr, "Token mismatch: expected (%s, %d) got (%s, %d)\n", expected_val, type, tok->value, tok->type);
    exit(1);
  }
}

typedef struct {
  const char* input;
  int expected_type;
  const char* expected_value;
} lexer_test_case_t;

lexer_test_case_t tests[] = {
  {"foo", TOKEN_ID, "foo"},
  {"=", TOKEN_EQUALS, "="},
  {"(", TOKEN_LPAREN, "("},
  {")", TOKEN_RPAREN, ")"},
  {"{", TOKEN_LBRACE, "{"},
  {"}", TOKEN_RBRACE, "}"},
  {"[", TOKEN_LBRACKET, "["},
  {"]", TOKEN_RBRACKET, "]"},
  {":", TOKEN_COLON, ":"},
  {",", TOKEN_COMMA, ","},
  {"<", TOKEN_LT, "<"},
  {">", TOKEN_GT, ">"},
  {"<=", TOKEN_LEQ, "<="},
  {">=", TOKEN_GEQ, ">="},
  {"->", TOKEN_ARROW_RIGHT, "->"},
  {"123", TOKEN_INT, "123"},
  {"\"hello\"", TOKEN_STRING, "hello"},
  {";", TOKEN_SEMICOLON, ";"},
  {"+", TOKEN_PLUS, "+"},
  {"-", TOKEN_MINUS, "-"},
  {"/", TOKEN_DIVIDE, "/"},
  {"*", TOKEN_MULTIPLY, "*"},
  {"<<", TOKEN_LSHIFT, "<<"},
  {">>", TOKEN_RSHIFT, ">>"},
  {"// comment", TOKEN_COMMENT, "//"},
  {"", TOKEN_EOF, NULL}
};

void test_each_type() {
  size_t n = sizeof(tests) / sizeof(tests[0]);
  printf("Testing all types, count: %zu\n", n);
  for (size_t i = 0; i < n; i++) {
    lexer_t* lexer = init_lexer((char*)tests[i].input);
    token_t* token = lexer_next_token(lexer);

    if (token->type != tests[i].expected_type) {
      printf("Test %zu failed: input '%s' expected token type %d got %d\n", i, tests[i].input, tests[i].expected_type, token->type);
    }

    if (tests[i].expected_value && token->value && strcmp(token->value, tests[i].expected_value) != 0) {
      printf("Test %zu failed: input '%s' expected token value '%s' got '%s'\n", i, tests[i].input, tests[i].expected_value, token->value);
    }

    free(token);
    free(lexer);
  }
}

void test_short_string() {
  char* src = "class myVar = 123;";
  lexer_t* lexer = init_lexer(src);

  printf("Testing src: %s\n", src);

  token_t* t1 = lexer_next_token(lexer);
  assert_token(t1, "class", TOKEN_ID);

  token_t* t2 = lexer_next_token(lexer);
  assert_token(t2, "myVar", TOKEN_ID);

  token_t* t3 = lexer_next_token(lexer);
  assert_token(t3, "=", TOKEN_EQUALS);

  token_t* t4 = lexer_next_token(lexer);
  assert_token(t4, "123", TOKEN_INT);

  token_t* t5 = lexer_next_token(lexer);
  assert_token(t5, ";", TOKEN_SEMICOLON);
}

int main() {
  test_each_type();
  test_short_string();
  printf("All tests passed!\n");
  return 0;
}
