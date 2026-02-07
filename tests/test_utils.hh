#ifndef TEST_UTILS_H_
#define TEST_UTILS_H_

#include <memory>
#include <sstream>
#include <string>

#include "lexer.hh"
#include "parser.hh"
#include "sema.hh"

inline std::unique_ptr<ProgramNode<NodeInfo>> parse_program(
    const std::string& source) {
  std::istringstream input(source);
  Lexer lexer(input);
  Parser parser(lexer);
  return std::unique_ptr<ProgramNode<NodeInfo>>(parser.parseProgram());
}

inline ProgramNode<NodeInfo>* analyze_program(ProgramNode<NodeInfo>& root) {
  Sema sema;
  return sema.analyze(root);
}

#endif  // TEST_UTILS_H_
