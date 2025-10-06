#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "gen.hh"
#include "lexer.hh"
#include "log.hh"
#include "parser.hh"
#include "sema.hh"

std::string file_contents(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file) {
    LOG_FATAL("Could not open file: {}", filepath);
    return "";
  }

  std::streamsize file_size = file.tellg();
  if (file_size < 0) {
    LOG_FATAL("Could not determine size of file: {}", filepath);
    return "";
  }

  std::string contents(file_size, '\0');
  file.seekg(0, std::ios::beg);
  if (!file.read(&contents[0], file_size)) {
    LOG_FATAL("Could not read contents of file: {}", filepath);
    return "";
  }

  return contents;
}

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

  std::ofstream out("out.s");
  out << code;
  out.close();

  delete ast;
  delete sema_tree;
}
