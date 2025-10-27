#include "visitor/treetransformer.hh"

#include <memory>

#include "ast.hh"
#include "log.hh"

ProgramNode<SemaExtra> *TreeTransformer::transform(
    ProgramNode<ParseExtra> &root) {
  TreeTransformer::visit(root);  // need to do more things in loop

  auto *transformed = dynamic_cast<ProgramNode<SemaExtra> *>(stmt_stack.top());
  stmt_stack.pop();
  return transformed;
}

void TreeTransformer::visit(ProgramNode<ParseExtra> &node) {
  uptr_vector<StmtNode<SemaExtra>> children;

  for (auto &child : node.children) {
    child->accept(*this);
    children.emplace_back(stmt_stack.top());
    stmt_stack.pop();
  }

  auto *program = new ProgramNode<SemaExtra>(std::move(children));
  stmt_stack.push(program);
}

void TreeTransformer::visit(VarDeclNode<ParseExtra> &node) {
  ExprNode<SemaExtra> *init = nullptr;

  if (node.initializer) {
    node.initializer->accept(*this);
    init = expr_stack.top();
    expr_stack.pop();
  }

  auto *decl = new VarDeclNode<SemaExtra>(node.type_token, node.identifier,
                                          init, node.location);

  decl->extra.resolved_type = node.type_token.getType();

  stmt_stack.push(decl);
}

void TreeTransformer::visit(LiteralExprNode<ParseExtra> &node) {
  auto *literal =
      new LiteralExprNode<SemaExtra>(node.literal_token, node.location);
  literal->extra.resolved_type = lookupType(&node);

  expr_stack.push(literal);
}

void TreeTransformer::visit(BinaryExprNode<ParseExtra> &node) {
  node.left->accept(*this);
  ExprNode<SemaExtra> *left = expr_stack.top();
  expr_stack.pop();

  node.right->accept(*this);
  ExprNode<SemaExtra> *right = expr_stack.top();
  expr_stack.pop();

  auto *binary =
      new BinaryExprNode<SemaExtra>(left, node.op, right, node.location);

  binary->extra.resolved_type = lookupType(&node);

  expr_stack.push(binary);
}

// Stub implementations for remaining visitors
void TreeTransformer::visit(ASTNode<ParseExtra> &node) {
  ASTVisitor<ParseExtra>::visit(node);
}

void TreeTransformer::visit(UnaryExprNode<ParseExtra> &node) {
  node.operand->accept(*this);
  ExprNode<SemaExtra> *operand = expr_stack.top();
  expr_stack.pop();

  auto *unary = new UnaryExprNode<SemaExtra>(node.op, operand, node.location);
  unary->extra.resolved_type = lookupType(&node);

  expr_stack.push(unary);
}

void TreeTransformer::visit(IdentifierExprNode<ParseExtra> &node) {
  LOG_DEBUG("[Tree] visited ident");
  auto *id = new IdentifierExprNode<SemaExtra>(node.identifier, node.location);
  id->extra.resolved_type = lookupType(&node);
  expr_stack.push(id);
  LOG_DEBUG("[Tree] finished ident");
}

void TreeTransformer::visit(AssignmentExprNode<ParseExtra> &node) {
  node.left->accept(*this);
  ExprNode<SemaExtra> *left_raw = expr_stack.top();
  expr_stack.pop();

  node.right->accept(*this);
  ExprNode<SemaExtra> *right_raw = expr_stack.top();
  expr_stack.pop();

  auto *assign = new AssignmentExprNode<SemaExtra>(left_raw, node.op, right_raw,
                                                   node.location);
  assign->extra.resolved_type = lookupType(&node);

  expr_stack.push(assign);
  LOG_DEBUG("[Tree] finished assignment");
}

void TreeTransformer::visit(MethodCallNode<ParseExtra> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(ArgumentNode<ParseExtra> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(ParamNode<ParseExtra> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(BlockNode<ParseExtra> &node) {
  uptr_vector<StmtNode<SemaExtra>> stmts;

  for (auto &stmt : node.statements) {
    stmt->accept(*this);
    stmts.emplace_back(stmt_stack.top());
    stmt_stack.pop();
  }

  auto *block = new BlockNode<SemaExtra>(std::move(stmts), node.location);
  stmt_stack.push(block);
}

void TreeTransformer::visit(IfStmtNode<ParseExtra> &node) {
  node.condition->accept(*this);
  ExprNode<SemaExtra> *cond = expr_stack.top();
  expr_stack.pop();

  node.statement->accept(*this);
  StmtNode<SemaExtra> *stmt = stmt_stack.top();
  stmt_stack.pop();

  StmtNode<SemaExtra> *else_stmt = nullptr;
  if (node.else_stmt) {
    node.else_stmt->accept(*this);
    else_stmt = stmt_stack.top();
    stmt_stack.pop();
  }

  auto *if_stmt =
      new IfStmtNode<SemaExtra>(cond, stmt, else_stmt, node.location);
  stmt_stack.push(if_stmt);
}

void TreeTransformer::visit(ReturnStmtNode<ParseExtra> &node) {
  node.ret->accept(*this);
  ExprNode<SemaExtra> *ret = expr_stack.top();
  expr_stack.pop();

  auto *return_stmt = new ReturnStmtNode<SemaExtra>(ret, node.location);
  stmt_stack.push(return_stmt);
}

void TreeTransformer::visit(ClassNode<ParseExtra> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(FieldDeclNode<ParseExtra> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(MethodDeclNode<ParseExtra> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(ConstructorDeclNode<ParseExtra> &) {
  // Stub: not implemented yet
}

void TreeTransformer::visit(ExprStmtNode<ParseExtra> &node) {
  LOG_DEBUG("[Tree] Visited exprstmt");
  node.expr->accept(*this);
  ExprNode<SemaExtra> *expr = expr_stack.top();
  expr_stack.pop();

  auto *expr_stmt = new ExprStmtNode<SemaExtra>(expr, node.location);
  stmt_stack.push(expr_stmt);
}
