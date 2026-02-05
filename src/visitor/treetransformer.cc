#include "visitor/treetransformer.hh"

#include <memory>

#include "ast.hh"
#include "log.hh"

ProgramNode<NodeInfo> *TreeTransformer::transform(
    ProgramNode<NodeInfo> &root) {
  TreeTransformer::visit(root);  // need to do more things in loop

  auto *transformed = dynamic_cast<ProgramNode<NodeInfo> *>(stmt_stack.top());
  stmt_stack.pop();
  return transformed;
}

void TreeTransformer::visit(ProgramNode<NodeInfo> &node) {
  uptr_vector<StmtNode<NodeInfo>> children;

  for (auto &child : node.children) {
    child->accept(*this);
    children.emplace_back(stmt_stack.top());
    stmt_stack.pop();
  }

  auto *program = new ProgramNode<NodeInfo>(std::move(children));
  stmt_stack.push(program);
}

void TreeTransformer::visit(VarDeclNode<NodeInfo> &node) {
  ExprNode<NodeInfo> *init = nullptr;

  if (node.initializer) {
    node.initializer->accept(*this);
    init = expr_stack.top();
    expr_stack.pop();
  }

  auto *decl = new VarDeclNode<NodeInfo>(node.type_token, node.identifier,
                                          init, node.location);

  decl->extra.resolved_type = node.type_token.getType();

  stmt_stack.push(decl);
}

void TreeTransformer::visit(LiteralExprNode<NodeInfo> &node) {
  auto *literal =
      new LiteralExprNode<NodeInfo>(node.literal_token, node.location);
  literal->extra.resolved_type = lookupType(&node).token_type;

  expr_stack.push(literal);
}

void TreeTransformer::visit(BinaryExprNode<NodeInfo> &node) {
  node.left->accept(*this);
  ExprNode<NodeInfo> *left = expr_stack.top();
  expr_stack.pop();

  node.right->accept(*this);
  ExprNode<NodeInfo> *right = expr_stack.top();
  expr_stack.pop();

  auto *binary =
      new BinaryExprNode<NodeInfo>(left, node.op, right, node.location);

  binary->extra.resolved_type = lookupType(&node).token_type;

  expr_stack.push(binary);
}

// Stub implementations for remaining visitors
void TreeTransformer::visit(ASTNode<NodeInfo> &node) {
  ASTVisitor<NodeInfo>::visit(node);
}

void TreeTransformer::visit(UnaryExprNode<NodeInfo> &node) {
  node.operand->accept(*this);
  ExprNode<NodeInfo> *operand = expr_stack.top();
  expr_stack.pop();

  auto *unary = new UnaryExprNode<NodeInfo>(node.op, operand, node.location);
  unary->extra.resolved_type = lookupType(&node).token_type;

  expr_stack.push(unary);
}

void TreeTransformer::visit(IdentifierExprNode<NodeInfo> &node) {
  LOG_DEBUG("[Tree] visited ident");
  auto *id = new IdentifierExprNode<NodeInfo>(node.identifier, node.location);
  id->extra.resolved_type = lookupType(&node).token_type;
  expr_stack.push(id);
  LOG_DEBUG("[Tree] finished ident");
}

void TreeTransformer::visit(AssignmentExprNode<NodeInfo> &node) {
  node.left->accept(*this);
  ExprNode<NodeInfo> *left_raw = expr_stack.top();
  expr_stack.pop();

  node.right->accept(*this);
  ExprNode<NodeInfo> *right_raw = expr_stack.top();
  expr_stack.pop();

  auto *assign = new AssignmentExprNode<NodeInfo>(left_raw, node.op, right_raw,
                                                   node.location);
  assign->extra.resolved_type = lookupType(&node).token_type;

  expr_stack.push(assign);
  LOG_DEBUG("[Tree] finished assignment");
}

void TreeTransformer::visit(MethodCallNode<NodeInfo> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(ArgumentNode<NodeInfo> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(ParamNode<NodeInfo> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(BlockNode<NodeInfo> &node) {
  uptr_vector<StmtNode<NodeInfo>> stmts;

  for (auto &stmt : node.statements) {
    stmt->accept(*this);
    stmts.emplace_back(stmt_stack.top());
    stmt_stack.pop();
  }

  auto *block = new BlockNode<NodeInfo>(std::move(stmts), node.location);
  stmt_stack.push(block);
}

void TreeTransformer::visit(IfStmtNode<NodeInfo> &node) {
  node.condition->accept(*this);
  ExprNode<NodeInfo> *cond = expr_stack.top();
  expr_stack.pop();

  node.statement->accept(*this);
  StmtNode<NodeInfo> *stmt = stmt_stack.top();
  stmt_stack.pop();

  StmtNode<NodeInfo> *else_stmt = nullptr;
  if (node.else_stmt) {
    node.else_stmt->accept(*this);
    else_stmt = stmt_stack.top();
    stmt_stack.pop();
  }

  auto *if_stmt =
      new IfStmtNode<NodeInfo>(cond, stmt, else_stmt, node.location);
  stmt_stack.push(if_stmt);
}

void TreeTransformer::visit(WhileStmtNode<NodeInfo> &node) {
  node.condition->accept(*this);
  ExprNode<NodeInfo> *cond = expr_stack.top();
  expr_stack.pop();

  node.statement->accept(*this);
  StmtNode<NodeInfo> *stmt = stmt_stack.top();
  stmt_stack.pop();

  auto *while_stmt = new WhileStmtNode<NodeInfo>(cond, stmt, node.location);
  stmt_stack.push(while_stmt);
}

void TreeTransformer::visit(ReturnStmtNode<NodeInfo> &node) {
  node.ret->accept(*this);
  ExprNode<NodeInfo> *ret = expr_stack.top();
  expr_stack.pop();

  auto *return_stmt = new ReturnStmtNode<NodeInfo>(ret, node.location);
  stmt_stack.push(return_stmt);
}

void TreeTransformer::visit(ClassNode<NodeInfo> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(FieldDeclNode<NodeInfo> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(MethodDeclNode<NodeInfo> &node) {
  // Stub: not implemented yet
  uptr_vector<ParamNode<NodeInfo>> param_list;
  for (auto &param : node.param_list) {
    param->accept(*this);
    param_list.emplace_back(
        dynamic_cast<ParamNode<NodeInfo> *>(stmt_stack.top()));
    stmt_stack.pop();
  }
  node.body->accept(*this);
  auto body_raw = dynamic_cast<BlockNode<NodeInfo> *>(stmt_stack.top());
  stmt_stack.pop();
  auto body = uptr<BlockNode<NodeInfo>>(body_raw);

  auto *methoddecl_stmt = new MethodDeclNode<NodeInfo>(
      node.access_modifier, node.is_static, node.type, node.identifier,
      std::move(param_list), std::move(body), node.location);
  stmt_stack.push(methoddecl_stmt);
}

void TreeTransformer::visit(ConstructorDeclNode<NodeInfo> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(ExprStmtNode<NodeInfo> &node) {
  LOG_DEBUG("[Tree] Visited exprstmt");
  node.expr->accept(*this);
  ExprNode<NodeInfo> *expr = expr_stack.top();
  expr_stack.pop();

  auto *expr_stmt = new ExprStmtNode<NodeInfo>(expr, node.location);
  stmt_stack.push(expr_stmt);
}
