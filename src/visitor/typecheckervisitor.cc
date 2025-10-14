#include "ast.hh"
#include "ast_utils.hh"
#include "log.hh"
#include "token.hh"
#include "visitor/typechecker.hh"

void TypeCheckerVisitor::visit(BinaryExprNode<ParseExtra>& node) {
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);

  TokenType left_type = get_expr_type(node.left.get());
  TokenType right_type = get_expr_type(node.right.get());

  if (left_type == TokenType::TOKEN_UNKNOWN ||
      right_type == TokenType::TOKEN_UNKNOWN) {
    set_expr_type(&node, TokenType::TOKEN_UNKNOWN);
    return;
  }

  TokenType result_type =
      check_binary_op(node.op.getType(), left_type, right_type);
  if (result_type == TokenType::TOKEN_UNKNOWN) {
    std::string error =
        "Invalid binary operation: " + ASTStringBuilder::node_to_string(&node);
    report_error(error, node.location);
  }

  set_expr_type(&node, result_type);
}

void TypeCheckerVisitor::visit(IdentifierExprNode<ParseExtra>& node) {
  std::string name = node.identifier.getValue();
  Symbol* symbol = lookup_symbol(name);

  if (!symbol) {
    report_error("Undeclared identifier '" + name + "'", node.location);
    set_expr_type(&node, TokenType::TOKEN_UNKNOWN);
    return;
  }

  set_expr_type(&node, symbol->type);
}

void TypeCheckerVisitor::visit(ProgramNode<ParseExtra>& node) {
  LOG_DEBUG("TypeCheckerVisitor: Visiting ProgramNode with {} children",
            node.children.size());
  // for (auto& child : node.children) {
  //   child->accept(*this);
  // }
}

void TypeCheckerVisitor::visit(VarDeclNode<ParseExtra>& node) {
  if (node.initializer) {
    node.initializer->accept(*this);
  }
}

void TypeCheckerVisitor::visit(LiteralExprNode<ParseExtra>& node) {
  TokenType literal_type = node.literal_token.getType();
  std::string value = node.literal_token.getValue();
  set_expr_type(&node, literal_type);
}

void TypeCheckerVisitor::visit(ExprStmtNode<ParseExtra>& node) {
  if (node.expr) node.expr->accept(*this);
}

void TypeCheckerVisitor::visit(AssignmentExprNode<ParseExtra>& node) {
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);

  if (node.op.getType() == TokenType::TOKEN_EQUALS) {
    if (auto* identifier =
            dynamic_cast<LiteralExprNode<ParseExtra>*>(node.left.get())) {
      if (lookup_symbol(identifier->literal_token.getValue())->type ==
          get_expr_type(node.right.get())) {
        LOG_DEBUG("[Type] Correct");
      } else {
        report_error("Tried to assign mismatching type", node.location);
      }
    }
  }
}
