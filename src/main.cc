#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "gen.hh"
#include "lexer.hh"
#include "log.hh"
#include "parser.hh"
#include "sema.hh"

void print_usage(char** argv) {
  LOG_FATAL("USAGE: {} <path-to-file>\n", argv[0]);
  exit(1);
}

int main(const int argc, char** argv) {
  if (argc != 2) print_usage(argv);

  std::string filepath = argv[1];

  std::ifstream file(filepath, std::ios::binary);
  if (!file) {
    LOG_FATAL("Could not open file: {}", filepath);
    exit(1);
  }

  Lexer lexer(file);

  Parser parser(lexer);

  ProgramNode<ParseExtra>* ast = parser.parseProgram();
  if (ast != nullptr) {
    Log::print_ast_reflection(ast);
  } else {
    LOG_ERROR("Parser returned null - no AST generated");
  }

  Sema sema;
  ProgramNode<SemaExtra>* sema_tree = sema.analyze(*ast);

  CodeGenerator gen;
  std::string code = gen.generate(*sema_tree);
  LOG_INFO("\n{}", code);

  std::ofstream out("program.s");
  out << code;
  out.close();

  delete ast;
  delete sema_tree;
}
