#ifndef SYMBOL_H_
#define SYMBOL_H_

#include <string>
#include <vector>

#include "sourcelocation.hh"

template <typename Extra>
struct TypeNode;
struct NodeInfo;

struct Symbol {
  std::string name;
  TypeNode<NodeInfo>* type;
  SourceLocation decl_loc;
  int stack_offset;
  bool has_stack_slot = false;

  std::string access_modifier = "private";
  bool is_static = false;

  std::string owner_class;
  std::string method_key;

  std::vector<TypeNode<NodeInfo>*> param_types;
  std::vector<std::string> param_names;
  std::vector<std::string> param_type_names;

  bool is_param = false;
  bool is_method = false;
  bool is_field = false;
  bool is_class = false;
};

#endif  // SYMBOL_H_
