#include "visitor/nameresolver.hh"

#include "ast.hh"
#include "log.hh"

const std::vector<Symbol> *NameResolver::find_method_overloads(
    const std::string &owner, const std::string &name) {
  if (!method_symbols) return nullptr;

  return method_symbols->find_all(owner, name);
}

void NameResolver::enter(BlockNode<NodeInfo> &) { push_scope(); }
void NameResolver::exit(BlockNode<NodeInfo> &) { pop_scope(); }

void NameResolver::enter(ProgramNode<NodeInfo> &) { push_scope(); }
void NameResolver::exit(ProgramNode<NodeInfo> &) { pop_scope(); }

void NameResolver::enter(MethodDeclNode<NodeInfo> &node) {
  enter_method(node.identifier.getValue(), node.type.getType());
  push_scope();

  for (auto &param : node.param_list) {
    if (!param) continue;
    Symbol param_sym;
    param_sym.name = param->identifier.getValue();
    param_sym.type = param->type.getType();
    param_sym.type_name = param->type.getValue();
    param_sym.is_param = true;
    param_sym.decl_loc = param->location;

    if (check_symbol(param_sym.name))
      report_error("Redeclaration of parameter '" + param_sym.name + "'",
                   param->location);
    else
      add_symbol(param_sym);
  }
}

void NameResolver::exit(MethodDeclNode<NodeInfo> &) {
  pop_scope();
  exit_method();
}

void NameResolver::enter(ConstructorDeclNode<NodeInfo> &node) {
  enter_method(node.identifier.getValue(), TokenType::TOKEN_UNKNOWN);
  push_scope();

  for (auto &param : node.param_list) {
    if (!param) continue;
    Symbol param_sym;
    param_sym.name = param->identifier.getValue();
    param_sym.type = param->type.getType();
    param_sym.type_name = param->type.getValue();
    param_sym.is_param = true;
    param_sym.decl_loc = param->location;

    if (check_symbol(param_sym.name))
      report_error("Redeclaration of parameter '" + param_sym.name + "'",
                   param->location);
    else
      add_symbol(param_sym);
  }
}
void NameResolver::exit(ConstructorDeclNode<NodeInfo> &) {
  pop_scope();
  exit_method();
}

void NameResolver::enter(ClassNode<NodeInfo> &node) {
  enter_class(node.identifier.getValue());
}

void NameResolver::exit(ClassNode<NodeInfo> &) { exit_class(); }

void NameResolver::visit(VarDeclNode<NodeInfo> &node) {
  const std::string name = node.identifier.getValue();
  if (check_symbol(name)) {
    report_error("Redeclaration of variable '" + name + "'", node.location);
    return;
  }

  Symbol var_sym;
  var_sym.name = name;
  var_sym.type = node.type_token.getType();
  var_sym.type_name = node.type_token.getValue();
  var_sym.decl_loc = node.location;

  add_symbol(var_sym);
  node.extra.sym = &scope_stack.back()[name];
}

void NameResolver::visit(ParamNode<NodeInfo> &node) {}

void NameResolver::visit(MethodDeclNode<NodeInfo> &node) {
  const std::string name = node.identifier.getValue();
  std::vector<TokenType> param_types;
  param_types.reserve(node.param_list.size());
  for (auto &param : node.param_list) {
    if (param) {
      param_types.push_back(param->type.getType());
    }
  }

  const std::string owner = current_class.empty() ? "<global>" : current_class;
  if (!method_symbols) {
    report_error("Missing method table for resolver", node.location);
    return;
  }

  std::vector<Symbol> overloads = *find_method_overloads(owner, name);
  if (overloads.empty()) {
    report_error("Missing method declaration for '" + name + "'",
                 node.location);
    return;
  }

  node.extra.overload_set = overloads;
}

void NameResolver::visit(IdentifierExprNode<NodeInfo> &node) {
  const std::string name = node.identifier.getValue();
  Symbol *symbol = lookup_symbol(name);
  if (!symbol) {
    report_error("Undeclared identifier '" + name + "'", node.location);
    return;
  }
  node.extra.sym = symbol;
}

void NameResolver::visit(ClassNode<NodeInfo> &node) {}

void NameResolver::visit(ArgumentNode<NodeInfo> &node) {
  if (node.expr) node.expr->accept(*this);
}

void NameResolver::visit(MethodCallNode<NodeInfo> &node) {
  LOG_DEBUG("[NAME] visiting method call");
  if (node.expr) node.expr->accept(*this);

  std::string owner;
  bool is_static = false;

  if (node.expr == nullptr)
    owner = "<global>";  // temp i think?

  else if (auto *ident =
               dynamic_cast<IdentifierExprNode<NodeInfo> *>(node.expr.get())) {
    Symbol *base_sym = ident->extra.sym;
    if (!base_sym) {
      report_error("Unresolved method base", node.location);
      return;
    }

    if (base_sym->is_class) {
      owner = base_sym->name;
      is_static = true;
    } else if (base_sym->type == TokenType::TOKEN_DATA_TYPE) {
      owner = base_sym->type_name;
    }
  } else {
    if (node.expr->extra.resolved_type == TokenType::TOKEN_DATA_TYPE) {
      owner = node.expr->extra.type_name;
    }
  }

  if (owner.empty()) {
    report_error("Method call target is not a class type", node.location);
    return;
  }

  const std::vector<Symbol> overloads =
      *find_method_overloads(owner, node.identifier.getValue());

  LOG_DEBUG("[NAME] method overloads size {}", overloads.size());
  if (overloads.empty()) {
    report_error("Method '" + node.identifier.getValue() + "' not found on '" +
                     owner + "'",
                 node.location);
    return;
  }

  node.extra.overload_set = overloads;
}

void NameResolver::visit(AssignmentExprNode<NodeInfo> &node) {}
