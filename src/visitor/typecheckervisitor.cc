#include "ast.hh"
#include "ast_utils.hh"
#include "log.hh"
#include "token.hh"
#include "visitor/typechecker.hh"

void TypeCheckerVisitor::visit(BinaryExprNode<NodeInfo>& node) {
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);

  TypeInfo left_type = get_expr_type(node.left.get());
  TypeInfo right_type = get_expr_type(node.right.get());

  if (left_type.token_type == TokenType::TOKEN_UNKNOWN ||
      right_type.token_type == TokenType::TOKEN_UNKNOWN) {
    set_expr_type(&node, TokenType::TOKEN_UNKNOWN);
    return;
  }

  TypeInfo result_type =
      check_binary_op(node.op.getType(), left_type, right_type);
  if (result_type.token_type == TokenType::TOKEN_UNKNOWN) {
    std::string error =
        "Invalid binary operation: " + ASTStringBuilder::node_to_string(&node);
    report_error(error, node.location);
  }

  set_expr_type(&node, result_type);
}

void TypeCheckerVisitor::visit(IdentifierExprNode<NodeInfo>& node) {
  std::string name = node.identifier.getValue();
  Symbol* symbol = lookup_symbol(name);

  if (!symbol) {
    report_error("Undeclared identifier '" + name + "'", node.location);
    set_expr_type(&node, TokenType::TOKEN_UNKNOWN);
    return;
  }

  if (symbol->is_class) {
    set_expr_type(&node, TokenType::TOKEN_DATA_TYPE, symbol->name);
  } else {
    std::string type_name = get_type_name_from_token(symbol->type);
    set_expr_type(&node, symbol->type, type_name);
  }
}

void TypeCheckerVisitor::visit(ProgramNode<NodeInfo>& node) {
  LOG_DEBUG("TypeCheckerVisitor: Visiting ProgramNode with {} children",
            node.children.size());
  // for (auto& child : node.children) {
  //   child->accept(*this);
  // }
}

void TypeCheckerVisitor::visit(VarDeclNode<NodeInfo>& node) {
  std::string declared_type = node.type_token.getValue();

  if (node.initializer) {
    node.initializer->accept(*this);
    TypeInfo initializer_type = get_expr_type(node.initializer.get());

    if (!types_compatible(declared_type, initializer_type)) {
      report_error("Tried to assign type '" + initializer_type.type_name +
                       "' to variable of type '" + declared_type + "'",
                   node.location);
    }
  }
}

void TypeCheckerVisitor::visit(MethodDeclNode<NodeInfo>& node) {
  std::string declared_type = node.type.getValue();
  // oh i need a way to see what type its actually returning
}

void TypeCheckerVisitor::visit(LiteralExprNode<NodeInfo>& node) {
  TokenType literal_type = node.literal_token.getType();
  std::string type_name = get_type_name_from_token(literal_type);
  set_expr_type(&node, literal_type, type_name);
}

void TypeCheckerVisitor::visit(ExprStmtNode<NodeInfo>& node) {
  if (node.expr) node.expr->accept(*this);
}

void TypeCheckerVisitor::visit(AssignmentExprNode<NodeInfo>& node) {
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);

  if (node.op.getType() == TokenType::TOKEN_EQUALS) {
    if (auto* identifier =
            dynamic_cast<LiteralExprNode<NodeInfo>*>(node.left.get())) {
      if (lookup_symbol(identifier->literal_token.getValue())->type ==
          get_expr_type(node.right.get()).token_type) {
        LOG_DEBUG("[Type] Correct");
      } else {
        report_error("Tried to assign mismatching type", node.location);
      }
    }
  }
}

void TypeCheckerVisitor::visit(UnaryExprNode<NodeInfo>& node) {
  node.operand->accept(*this);
  TokenType operand_type = get_expr_type(node.operand.get()).token_type;

  switch (node.op.getType()) {
    case TokenType::TOKEN_MINUS: {
      if (operand_type != TokenType::TOKEN_INT) {
        report_error("Unary '-' requires numeric type", node.location);
      }
      set_expr_type(&node, operand_type);
      break;
    }
    default:
      report_error("Unknown unary operator", node.location);
  }
}
