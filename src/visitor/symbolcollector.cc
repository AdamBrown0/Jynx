#include "ast.hh"
#include "ast_utils.hh"
#include "log.hh"
#include "visitor/symbolcollector.hh"

void SymbolCollectorVisitor::visit(VarDeclNode<NodeInfo> &node) {
  Symbol var_symbol;
  var_symbol.name = node.identifier.getValue();
  var_symbol.type = node.type_token.getType();
  var_symbol.type_name = node.type_token.getValue();
  var_symbol.decl_loc = node.location;

  add_symbol(var_symbol);
}

void SymbolCollectorVisitor::visit(MethodDeclNode<NodeInfo> &node) {
  (void)node;
}

void SymbolCollectorVisitor::visit(ParamNode<NodeInfo> &node) {
  Symbol param_symbol;
  param_symbol.name = node.identifier.getValue();
  param_symbol.type = node.type.getType();
  param_symbol.type_name = node.type.getValue();
  param_symbol.is_param = true;
  param_symbol.decl_loc = node.location;

  add_symbol(param_symbol);
}

void SymbolCollectorVisitor::visit(ClassNode<NodeInfo> &node) { (void)node; }
void SymbolCollectorVisitor::visit(FieldDeclNode<NodeInfo> &node) { (void)node; }
void SymbolCollectorVisitor::visit(ConstructorDeclNode<NodeInfo> &node) { (void)node; }

void SymbolCollectorVisitor::enter(BlockNode<NodeInfo> &) { push_scope(); }

void SymbolCollectorVisitor::exit(BlockNode<NodeInfo> &) { pop_scope(); }

void SymbolCollectorVisitor::enter(MethodDeclNode<NodeInfo> &node) {
  Symbol method_symbol;
  method_symbol.name = node.identifier.getValue();
  method_symbol.type = node.type.getType();
  method_symbol.type_name = node.type.getValue();
  method_symbol.decl_loc = node.location;
  method_symbol.access_modifier = node.access_modifier.getValue();
  method_symbol.is_method = true;

  if (!node.param_list.empty()) {
    for (auto &param : node.param_list) {
      method_symbol.param_types.emplace_back(param.get()->type.getType());
      method_symbol.param_names.emplace_back(
          param.get()->identifier.getValue());
    }
  }

  add_symbol(method_symbol);
  enter_method(node.identifier.getValue(), node.type.getType());
  push_scope();
}

void SymbolCollectorVisitor::exit(MethodDeclNode<NodeInfo> &) {
  pop_scope();
  exit_method();
}

void SymbolCollectorVisitor::enter(ConstructorDeclNode<NodeInfo> &node) {
  enter_method(node.identifier.getValue(), TokenType::TOKEN_UNKNOWN);
  push_scope();
}

void SymbolCollectorVisitor::exit(ConstructorDeclNode<NodeInfo> &) {
  pop_scope();
  exit_method();
}
