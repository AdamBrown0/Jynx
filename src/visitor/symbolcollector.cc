#include "visitor/symbolcollector.hh"

#include "ast.hh"
#include "ast_utils.hh"
#include "log.hh"

TokenType SymbolCollectorVisitor::resolve_param_type(const std::string &name) {
  if (name == "int") return TokenType::TOKEN_INT;
  if (name == "string") return TokenType::TOKEN_STRING;
  if (name == "bool")
    return TokenType::TOKEN_INT;  // assuming bool is int for now

  // For user-defined types, we'd look up the class, but for now return
  // DATA_TYPE
  return TokenType::TOKEN_DATA_TYPE;
}

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
  node.extra.sym = std::make_unique<Symbol>(param_symbol);
}

void SymbolCollectorVisitor::visit(ArgumentNode<NodeInfo> &node) {
  if (node.expr) node.expr->accept(*this);

  Symbol arg_symbol;
  arg_symbol.name = node.expr->extra.sym->name;
  arg_symbol.type = node.expr->extra.sym->type;
  arg_symbol.type_name = node.expr->extra.sym->type_name;
  arg_symbol.decl_loc = node.location;

  add_symbol(arg_symbol);
  node.extra.sym = std::make_unique<Symbol>(arg_symbol);
}

void SymbolCollectorVisitor::visit(LiteralExprNode<NodeInfo> &node) {
  Symbol lit_sym;
  lit_sym.type = node.literal_token.getType();
  lit_sym.decl_loc = node.location;

  add_symbol(lit_sym);
  node.extra.sym = std::make_unique<Symbol>(lit_sym);
}

void SymbolCollectorVisitor::visit(ClassNode<NodeInfo> &node) {
  Symbol class_sym;
  class_sym.name = node.identifier.getValue();
  class_sym.type = TokenType::KW_CLASS;
  class_sym.type_name = node.identifier.getValue();
  class_sym.decl_loc = node.location;
  class_sym.is_class = true;

  add_symbol(class_sym);
}

void SymbolCollectorVisitor::visit(FieldDeclNode<NodeInfo> &node) {
  (void)node;
}
void SymbolCollectorVisitor::visit(ConstructorDeclNode<NodeInfo> &node) {
  (void)node;
}

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
  method_symbol.owner_class = current_class.empty() ? "global" : current_class;

  if (!node.param_list.empty()) {
    for (auto &param : node.param_list) {
      TokenType resolved_type =
          resolve_param_type(param.get()->type.getValue());
      method_symbol.param_types.emplace_back(resolved_type);
      method_symbol.param_names.emplace_back(
          param.get()->identifier.getValue());
      method_symbol.param_type_names.emplace_back(param.get()->type.getValue());
    }
  }

  method_symbol.method_key = MethodTable::make_method_key(method_symbol);
  context.method_table.add_method(method_symbol);

  node.extra.sym = std::make_unique<Symbol>(method_symbol);
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
