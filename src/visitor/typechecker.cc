#include "visitor/typechecker.hh"

#include "ast.hh"
#include "ast_utils.hh"
#include "log.hh"
#include "token.hh"

void TypeCheckerVisitor::visit(BinaryExprNode<NodeInfo>& node) {
  TypeInfo left_type{node.left->extra.resolved_type,
                     node.left->extra.type_name};
  TypeInfo right_type{node.right->extra.resolved_type,
                      node.right->extra.type_name};

  if (left_type.token_type == TokenType::TOKEN_UNKNOWN ||
      right_type.token_type == TokenType::TOKEN_UNKNOWN) {
    report_error("BinaryExprNode left/right had unknown token type",
                 node.location);
    return;
  }

  TypeInfo result_type =
      check_binary_op(node.op.getType(), left_type, right_type);
  if (result_type.token_type == TokenType::TOKEN_UNKNOWN) {
    std::string error =
        "Invalid binary operation: " + ASTStringBuilder::node_to_string(&node);
    report_error(error, node.location);
  }

  // set_expr_type(&node, result_type);
  node.extra.resolved_type = result_type.token_type;
  node.extra.type_name = result_type.type_name;
}

void TypeCheckerVisitor::visit(IdentifierExprNode<NodeInfo>& node) {
  std::string name = node.identifier.getValue();
  Symbol* symbol = lookup_symbol(name);

  if (!symbol) {
    report_error("Undeclared identifier '" + name + "'", node.location);
    // set_expr_type(&node, TokenType::TOKEN_UNKNOWN);
    return;
  }

  if (symbol->is_class) {
    // set_expr_type(&node, TokenType::TOKEN_DATA_TYPE, symbol->name);
    node.extra.resolved_type = TokenType::TOKEN_DATA_TYPE;
    node.extra.type_name = symbol->name;
  } else {
    if (symbol->type == TokenType::TOKEN_DATA_TYPE) {
      TokenType resolved = resolve_type(symbol->type_name);
      node.extra.resolved_type = resolved;
      if (resolved == TokenType::TOKEN_DATA_TYPE) {
        node.extra.type_name = symbol->type_name;
      } else {
        node.extra.type_name = get_type_name_from_token(resolved);
      }
    } else {
      std::string type_name = get_type_name_from_token(symbol->type);
      // set_expr_type(&node, symbol->type, type_name);
      node.extra.resolved_type = symbol->type;
      node.extra.type_name = type_name;
    }
  }
}

void TypeCheckerVisitor::visit(VarDeclNode<NodeInfo>& node) {
  std::string declared_type = node.type_token.getValue();

  node.extra.resolved_type = resolve_type(node.type_token.getValue());
  node.extra.type_name = node.type_token.getValue();

  if (node.initializer) {
    TypeInfo initializer_type{node.initializer->extra.resolved_type,
                              node.initializer->extra.type_name};

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
  // set_expr_type(&node, literal_type, type_name);
  node.extra.resolved_type = literal_type;
  node.extra.type_name = type_name;
}

void TypeCheckerVisitor::visit(ExprStmtNode<NodeInfo>& node) { (void)node; }

void TypeCheckerVisitor::visit(AssignmentExprNode<NodeInfo>& node) {
  if (node.op.getType() == TokenType::TOKEN_EQUALS) {
    if (auto* identifier =
            dynamic_cast<IdentifierExprNode<NodeInfo>*>(node.left.get())) {
      Symbol* symbol = lookup_symbol(identifier->identifier.getValue());
      if (!symbol) {
        report_error("Undeclared identifier '" +
                         identifier->identifier.getValue() + "'",
                     node.location);
        return;
      }

      TokenType left_type = symbol->type;
      if (left_type == TokenType::TOKEN_DATA_TYPE) {
        left_type = resolve_type(symbol->type_name);
        if (left_type == TokenType::TOKEN_DATA_TYPE) {
          if (symbol->type_name == node.right->extra.type_name) {
            node.extra.resolved_type = node.right->extra.resolved_type;
            node.extra.type_name = node.right->extra.type_name;
            return;
          }
        }
      }

      if (left_type == node.right->extra.resolved_type) {
        node.extra.resolved_type = node.right->extra.resolved_type;
        node.extra.type_name = node.right->extra.type_name;
      } else {
        report_error("Tried to assign mismatching type", node.location);
      }
    }
  }
}

void TypeCheckerVisitor::visit(UnaryExprNode<NodeInfo>& node) {
  TokenType operand_type = node.operand->extra.resolved_type;

  switch (node.op.getType()) {
    case TokenType::TOKEN_MINUS: {
      if (operand_type != TokenType::TOKEN_INT) {
        report_error("Unary '-' requires numeric type", node.location);
      }
      // set_expr_type(&node, operand_type);
      node.extra.resolved_type = operand_type;
      break;
    }
    default:
      report_error("Unknown unary operator", node.location);
  }
}
