#include "ast.hh"
#include "ast_utils.hh"
#include "log.hh"
#include "visitor/symbolcollector.hh"

void SymbolCollectorVisitor::visit(VarDeclNode<ParseExtra> &node) {
  LOG_DEBUG("COLLECTING VARDECL");
  Symbol var_symbol;
  var_symbol.name = node.identifier.getValue();
  var_symbol.type = node.type_token.getType();
  var_symbol.decl_loc = node.location;

  add_symbol(var_symbol);

  if (node.initializer) {
    LOG_DEBUG("Initializer found");
    node.initializer->accept(*this);
  }
}

void SymbolCollectorVisitor::visit(BinaryExprNode<ParseExtra> &node) {
  LOG_DEBUG("COLLECTING BINARY");
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);
}

void SymbolCollectorVisitor::visit(ProgramNode<ParseExtra> &node) {
  LOG_DEBUG("SymbolCollectorVisitor: Visiting ProgramNode with {} children", node.children.size());
  // for (auto& child : node.children) {
  //   child->accept(*this);
  // }
}
