#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "diagnostics.hh"
#include "ir/gen.hh"
#include "lexer.hh"
#include "log.hh"
#include "parser.hh"
#include "sema.hh"

//
#include "ir/module.hh"
#include "ir/printer.hh"

void print_usage(char** argv) {
  LOG_FATAL("USAGE: {} <path-to-file>\n", argv[0]);
  exit(1);
}

int main(const int argc, char** argv) {
  Diagnostics::instance().clear();
  if (argc != 2) print_usage(argv);

  std::string filepath = argv[1];

  std::ifstream file(filepath, std::ios::binary);
  if (!file) {
    LOG_FATAL("Could not open file: {}", filepath);
    exit(1);
  }

  Lexer lexer(file, CompilerContext::instance());

  Parser parser(lexer, CompilerContext::instance());

  std::unique_ptr<ProgramNode> ast(parser.parseProgram());
  if (ast != nullptr) {
    Log::print_ast_reflection(ast.get());
  } else {
    LOG_ERROR("Parser returned null - no AST generated");
    return 1;
  }

  if (Diagnostics::instance().has_errors()) {
    Diagnostics::instance().print_errors();
    return 1;
  }

  Sema sema(CompilerContext::instance());
  ProgramNode* sema_tree = sema.analyze(*ast);

  if (!sema_tree) {
    if (Diagnostics::instance().has_errors()) {
      Diagnostics::instance().print_errors();
      LOG_ERROR("Semantic analysis failed; skipping code generation");
      // return 1;
    }
  }

  IRGenerator generator(CompilerContext::instance());

  ir::Module* module = generator.generateIR(*sema_tree);

  for (const auto& function : module->get_functions()) {
    IRPrinter::print(*function);
  }

  if (Diagnostics::instance().has_errors()) {
    Diagnostics::instance().print_errors();
  }

  delete module;
  return 0;
}
