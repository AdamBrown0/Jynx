#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "lexer.hh"
#include "parser.hh"

void print_usage(char** argv) {
  std::printf("USAGE: %s <path-to-file>\n", argv[0]);
  exit(1);
}

int main(const int argc, char** argv) {
  if (argc != 2) print_usage(argv);

  std::string filepath = argv[1];

  std::ifstream file(filepath, std::ios::binary);
  if (!file) {
    std::cerr << "Could not open file: " << filepath << std::endl;
    exit(1);
  }

  Lexer lexer(file);

  // while (true) {
  //   std::optional<Token> tok_opt = lexer.next_token();
  //   if (!tok_opt.has_value()) break;

  //   Token tok = tok_opt.value();

  //   tok.print();
  //   std::cout << std::endl;

  //   if (tok.getType() == TokenType::TOKEN_EOF) break;
  // }
  //

  Parser parser(lexer);

  VarDeclNode* node = (VarDeclNode*)parser.parseProgram();
  std::cout << node->name << std::endl;
}
