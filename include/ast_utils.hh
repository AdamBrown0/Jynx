#ifndef AST_UTILS_H_
#define AST_UTILS_H_

#include <string>

#include "ast.hh"

class ASTStringBuilder {
 public:
  template <typename Extra>
  static std::string node_to_string(ASTNode<Extra>* node) {
    if (!node) return "<null>";

    // ============ Expression Nodes ============
    if (auto literal = dynamic_cast<LiteralExprNode<Extra>*>(node)) {
      return literal->literal_token.getValue();
    } else if (auto identifier =
                   dynamic_cast<IdentifierExprNode<Extra>*>(node)) {
      return identifier->identifier.getValue();
    } else if (auto binary = dynamic_cast<BinaryExprNode<Extra>*>(node)) {
      return "(" + node_to_string(binary->left.get()) + " " +
             binary->op.getValue() + " " + node_to_string(binary->right.get()) +
             ")";
    } else if (auto unary = dynamic_cast<UnaryExprNode<Extra>*>(node)) {
      return unary->op.getValue() + node_to_string(unary->operand.get());
    } else if (auto assignment =
                   dynamic_cast<AssignmentExprNode<Extra>*>(node)) {
      return node_to_string(assignment->left.get()) + " " +
             assignment->op.getValue() + " " +
             node_to_string(assignment->right.get());
    } else if (auto method_call = dynamic_cast<MethodCallNode<Extra>*>(node)) {
      std::string result = node_to_string(method_call->expr.get()) + "." +
                           method_call->identifier.getValue() + "(";

      for (size_t i = 0; i < method_call->arg_list.size(); ++i) {
        if (i > 0) result += ", ";
        result += node_to_string(method_call->arg_list[i]->expr.get());
      }
      result += ")";
      return result;
    }

    // ============ Statement Nodes ============
    else if (auto var_decl = dynamic_cast<VarDeclNode<Extra>*>(node)) {
      std::string result = var_decl->type_token.getValue() + " " +
                           var_decl->identifier.getValue();
      if (var_decl->initializer) {
        result += " = " + node_to_string(var_decl->initializer.get());
      }
      return result;
    } else if (auto program = dynamic_cast<ProgramNode<Extra>*>(node)) {
      return "Program{" + std::to_string(program->children.size()) +
             " statements}";
    } else if (auto block = dynamic_cast<BlockNode<Extra>*>(node)) {
      return "Block{" + std::to_string(block->statements.size()) +
             " statements}";
    } else if (auto if_stmt = dynamic_cast<IfStmtNode<Extra>*>(node)) {
      std::string result =
          "if (" + node_to_string(if_stmt->condition.get()) + ")";
      if (if_stmt->else_stmt) {
        result += " else ...";
      }
      return result;
    } else if (auto while_stmt = dynamic_cast<WhileStmtNode<Extra>*>(node)) {
      return "while (" + node_to_string(while_stmt->condition.get()) + ")";
    } else if (auto return_stmt = dynamic_cast<ReturnStmtNode<Extra>*>(node)) {
      if (return_stmt->ret) {
        return "return " + node_to_string(return_stmt->ret.get());
      }
      return "return";
    } else if (auto expr_stmt = dynamic_cast<ExprStmtNode<Extra>*>(node)) {
      if (expr_stmt->expr) {
        return node_to_string(expr_stmt->expr.get()) + ";";
      }
      return "empty_statement;";
    }

    // ============ Class-related Nodes ============
    else if (auto class_node = dynamic_cast<ClassNode<Extra>*>(node)) {
      return "class " + class_node->identifier.getValue() + "{" +
             std::to_string(class_node->members.size()) + " members}";
    } else if (auto field_decl = dynamic_cast<FieldDeclNode<Extra>*>(node)) {
      std::string result = field_decl->access_modifier.getValue();
      if (field_decl->is_static) result += " static";
      result += " " + field_decl->type.getValue() + " " +
                field_decl->identifier.getValue();
      return result;
    } else if (auto method_decl = dynamic_cast<MethodDeclNode<Extra>*>(node)) {
      std::string result = method_decl->access_modifier.getValue();
      if (method_decl->is_static) result += " static";
      result += " " + method_decl->type.getValue() + " " +
                method_decl->identifier.getValue() + "(" +
                std::to_string(method_decl->param_list.size()) + " params)";
      return result;
    } else if (auto constructor_decl =
                   dynamic_cast<ConstructorDeclNode<Extra>*>(node)) {
      return constructor_decl->identifier.getValue() + "(" +
             std::to_string(constructor_decl->param_list.size()) + " params)";
    } else if (auto param = dynamic_cast<ParamNode<Extra>*>(node)) {
      return param->type.getValue() + " " + param->identifier.getValue();
    } else if (auto arg = dynamic_cast<ArgumentNode<Extra>*>(node)) {
      return node_to_string(arg->expr.get());
    }

    // ============ Fallback ============
    return "<unknown_node_type>";
  }

  // Utility method to get a short description of a node type
  template <typename Extra>
  static std::string node_type_name(ASTNode<Extra>* node) {
    if (!node) return "null";

    if (dynamic_cast<LiteralExprNode<Extra>*>(node)) return "Literal";
    if (dynamic_cast<IdentifierExprNode<Extra>*>(node)) return "Identifier";
    if (dynamic_cast<BinaryExprNode<Extra>*>(node)) return "BinaryExpr";
    if (dynamic_cast<UnaryExprNode<Extra>*>(node)) return "UnaryExpr";
    if (dynamic_cast<AssignmentExprNode<Extra>*>(node)) return "Assignment";
    if (dynamic_cast<MethodCallNode<Extra>*>(node)) return "MethodCall";
    if (dynamic_cast<VarDeclNode<Extra>*>(node)) return "VarDecl";
    if (dynamic_cast<ProgramNode<Extra>*>(node)) return "Program";
    if (dynamic_cast<BlockNode<Extra>*>(node)) return "Block";
    if (dynamic_cast<IfStmtNode<Extra>*>(node)) return "IfStmt";
    if (dynamic_cast<WhileStmtNode<Extra>*>(node)) return "WhileStmt";
    if (dynamic_cast<ReturnStmtNode<Extra>*>(node)) return "ReturnStmt";
    if (dynamic_cast<ExprStmtNode<Extra>*>(node)) return "ExprStmt";
    if (dynamic_cast<ClassNode<Extra>*>(node)) return "Class";
    if (dynamic_cast<FieldDeclNode<Extra>*>(node)) return "FieldDecl";
    if (dynamic_cast<MethodDeclNode<Extra>*>(node)) return "MethodDecl";
    if (dynamic_cast<ConstructorDeclNode<Extra>*>(node))
      return "ConstructorDecl";
    if (dynamic_cast<ParamNode<Extra>*>(node)) return "Param";
    if (dynamic_cast<ArgumentNode<Extra>*>(node)) return "Argument";

    return "Unknown";
  }

  // Method to get a detailed string representation for error messages
  template <typename Extra>
  static std::string detailed_node_info(ASTNode<Extra>* node) {
    if (!node) return "null";

    return node_type_name(node) + ": " + node_to_string(node);
  }
};

#endif  // AST_UTILS_H_
