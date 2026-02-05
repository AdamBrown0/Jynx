#include "ast.hh"
#include "ast_utils.hh"
#include "log.hh"
#include "visitor/symbolcollector.hh"

void SymbolCollectorVisitor::visit(VarDeclNode<NodeInfo> &node) {
  Symbol var_symbol;
  var_symbol.name = node.identifier.getValue();
  var_symbol.type = node.type_token.getType();
  var_symbol.decl_loc = node.location;

  add_symbol(var_symbol);

  if (node.initializer) {
    node.initializer->accept(*this);
  }
}

void SymbolCollectorVisitor::visit(MethodDeclNode<NodeInfo> &node) {
  Symbol method_symbol;
  method_symbol.name = node.identifier.getValue();
  method_symbol.type = node.type.getType();
  method_symbol.decl_loc = node.location;
  method_symbol.access_modifier = node.access_modifier.getValue();
  method_symbol.is_method = true;

  // params
  if (!node.param_list.empty()) {
    for (auto &param : node.param_list) {
      param->accept(*this);
      method_symbol.param_types.emplace_back(param.get()->type.getType());
      method_symbol.param_names.emplace_back(
          param.get()->identifier.getValue());
    }
  }

  add_symbol(method_symbol);
}

void SymbolCollectorVisitor::visit(ParamNode<NodeInfo> &node) {
  Symbol param_symbol;
  param_symbol.name = node.identifier.getValue();
  param_symbol.type = node.type.getType();
  param_symbol.is_param = true;
  param_symbol.decl_loc = node.location;

  add_symbol(param_symbol);
}

void SymbolCollectorVisitor::visit(BinaryExprNode<NodeInfo> &node) {
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);
}

void SymbolCollectorVisitor::visit(ProgramNode<NodeInfo> &node) {
  LOG_DEBUG("SymbolCollectorVisitor: Visiting ProgramNode with {} children",
            node.children.size());
  // for (auto& child : node.children) {
  //   child->accept(*this);
  // }
}

void SymbolCollectorVisitor::visit(BlockNode<NodeInfo> &node) {
  push_scope();
  for (auto &stmt : node.statements) stmt->accept(*this);
  pop_scope();
}

void SymbolCollectorVisitor::visit(ExprStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[Sym] ExprStmtNode");
  node.expr->accept(*this);
}

void SymbolCollectorVisitor::visit(AssignmentExprNode<NodeInfo> &node) {
  LOG_DEBUG("[Sym] AssignmentExprNode");
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);

  if (node.op.getType() == TokenType::TOKEN_EQUALS) {
    if (auto *identifier =
            dynamic_cast<IdentifierExprNode<NodeInfo> *>(node.left.get())) {
      if (check_symbol(identifier->identifier.getValue())) {
        LOG_DEBUG("[Sym] Found");
      } else {
        report_error("Attempted to assign value to invalid variable",
                     node.location);
      }
    } else {
      report_error("Expected identifier to assign to", node.location);
    }
  }
}
