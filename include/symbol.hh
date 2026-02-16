#ifndef SYMBOL_H_
#define SYMBOL_H_

#include <string>
#include <vector>

#include "token.hh"

struct Symbol {
  std::string name;
  TokenType type;
  std::string type_name;
  SourceLocation decl_loc;
  int stack_offset;
  bool has_stack_slot = false;

  std::string access_modifier = "private";
  bool is_static = false;

  std::string owner_class;
  std::string method_key;

  std::vector<TokenType> param_types;
  std::vector<std::string> param_names;
  std::vector<std::string> param_type_names;

  bool is_param = false;
  bool is_method = false;
  bool is_field = false;
  bool is_class = false;
};

#endif  // SYMBOL_H_
