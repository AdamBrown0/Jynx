#include "ast.hh"
#include "ast_utils.hh"
#include "log.hh"
#include "visitor/symbolcollector.hh"

void SymbolCollectorVisitor::visit(VarDeclNode<ParseExtra> &node) {
  Symbol var_symbol;
  var_symbol.name = node.identifier.getValue();
  var_symbol.type = node.type_token.getType();
  var_symbol.decl_loc = node.location;

  add_symbol(var_symbol);

  if (node.initializer) {
    node.initializer->accept(*this);
  }
}

void SymbolCollectorVisitor::visit(BinaryExprNode<ParseExtra> &node) {
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);
}

void SymbolCollectorVisitor::visit(ProgramNode<ParseExtra> &node) {
  LOG_DEBUG("SymbolCollectorVisitor: Visiting ProgramNode with {} children",
            node.children.size());
  // for (auto& child : node.children) {
  //   child->accept(*this);
  // }
}

void SymbolCollectorVisitor::visit(ExprStmtNode<ParseExtra> &node) {
  LOG_DEBUG("[Sym] ExprStmtNode");
  node.expr->accept(*this);
}

void SymbolCollectorVisitor::visit(AssignmentExprNode<ParseExtra> &node) {
  LOG_DEBUG("[Sym] AssignmentExprNode");
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);

  if (node.op.getType() == TokenType::TOKEN_EQUALS) {
    if (auto *identifier =
            dynamic_cast<LiteralExprNode<ParseExtra> *>(node.left.get())) {
      if (check_symbol(identifier->literal_token.getValue())) {
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
