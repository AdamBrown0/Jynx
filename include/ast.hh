#ifndef AST_H_
#define AST_H_

#include <memory>
#include <string>
#include <vector>

#include "log.hh"
#include "symbol.hh"
#include "token.hh"
#include "visitor/visitor.hh"

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T>
using uptr_vector = std::vector<uptr<T>>;

// Forward declarations
template <typename Extra>
struct ExprNode;
template <typename Extra>
struct StmtNode;
template <typename Extra>
struct ClassMemberNode;
template <typename Extra>
struct ParamNode;
template <typename Extra>
struct BlockNode;
template <typename Extra>
struct ArgumentNode;
template <typename Extra>
struct ElseStmtNode;

template <typename Extra>
struct ASTNode {
  SourceLocation location;
  Extra extra;

  ASTNode(SourceLocation loc) : location(loc), extra() {}
  virtual ~ASTNode() = default;

  virtual void accept(ASTVisitor<Extra>& visitor) { visitor.visit(*this); }
};

/// ============
/// Expressions
/// ============
template <typename Extra>
struct ExprNode : ASTNode<Extra> {
  ExprNode(SourceLocation loc) : ASTNode<Extra>(loc) {}

  void accept(ASTVisitor<Extra>& visitor) override { visitor.visit(*this); }
};

template <typename Extra>
struct BinaryExprNode : ExprNode<Extra> {
  uptr<ExprNode<Extra>> left;
  Token op;
  uptr<ExprNode<Extra>> right;

  BinaryExprNode(ExprNode<Extra>* left, Token op, ExprNode<Extra>* right,
                 SourceLocation loc)
      : ExprNode<Extra>(loc), left(left), op(op), right(right) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (left) left->accept(visitor);
    if (right) right->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct UnaryExprNode : ExprNode<Extra> {
  Token op;
  uptr<ExprNode<Extra>> operand;

  UnaryExprNode(Token op, ExprNode<Extra>* operand, SourceLocation loc)
      : ExprNode<Extra>(loc), op(op), operand(operand) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (operand) operand->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct LiteralExprNode : ExprNode<Extra> {
  Token literal_token;

  LiteralExprNode(Token token, SourceLocation loc)
      : ExprNode<Extra>(loc), literal_token(token) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct IdentifierExprNode : ExprNode<Extra> {
  Token identifier;

  IdentifierExprNode(Token identifier, SourceLocation loc)
      : ExprNode<Extra>(loc), identifier(identifier) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct AssignmentExprNode : ExprNode<Extra> {
  uptr<ExprNode<Extra>> left;
  Token op;
  uptr<ExprNode<Extra>> right;

  AssignmentExprNode(ExprNode<Extra>* left, Token op, ExprNode<Extra>* right,
                     SourceLocation loc)
      : ExprNode<Extra>(loc), left(left), op(op), right(right) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (left) left->accept(visitor);
    if (right) right->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct MethodCallNode : ExprNode<Extra> {
  uptr<ExprNode<Extra>> expr;
  Token identifier;
  uptr_vector<ArgumentNode<Extra>> arg_list;

  MethodCallNode(uptr<ExprNode<Extra>>&& expr, Token identifier,
                 uptr_vector<ArgumentNode<Extra>>&& arg_list,
                 SourceLocation loc)
      : ExprNode<Extra>(loc),
        expr(std::move(expr)),
        identifier(identifier),
        arg_list(std::move(arg_list)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (expr) expr->accept(visitor);
    for (auto& arg : arg_list) {
      if (arg) arg->accept(visitor);
    }
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

/// ============
/// Supporting Nodes
/// ============
template <typename Extra>
struct ArgumentNode : ASTNode<Extra> {
  uptr<ExprNode<Extra>> expr;

  ArgumentNode(uptr<ExprNode<Extra>> expr, SourceLocation loc)
      : ASTNode<Extra>(loc), expr(std::move(expr)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (expr) expr->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct ParamNode : ASTNode<Extra> {
  Token type;
  Token identifier;  // might actually want to replace with something like
                     // vardecl or variable node

  ParamNode(Token type, Token identifier, SourceLocation loc)
      : ASTNode<Extra>(loc), type(type), identifier(identifier) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

/// ============
/// Statements
/// ============
template <typename Extra>
struct StmtNode : ASTNode<Extra> {
  StmtNode(SourceLocation loc) : ASTNode<Extra>(loc) {}

  void accept(ASTVisitor<Extra>& visitor) override { visitor.visit(*this); }
};

template <typename Extra>
struct ProgramNode : StmtNode<Extra> {
  uptr_vector<StmtNode<Extra>> children;

  ProgramNode() : StmtNode<Extra>(SourceLocation()) {}
  ProgramNode(uptr_vector<StmtNode<Extra>>&& children)
      : StmtNode<Extra>(SourceLocation()), children(std::move(children)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    for (const auto& child : children) {
      if (child) child->accept(visitor);
    }
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct BlockNode : StmtNode<Extra> {
  uptr_vector<StmtNode<Extra>> statements;

  BlockNode(uptr_vector<StmtNode<Extra>>&& statements, SourceLocation loc)
      : StmtNode<Extra>(loc), statements(std::move(statements)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    for (auto& stmt : statements) {
      if (stmt) stmt->accept(visitor);
    }
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct VarDeclNode : StmtNode<Extra> {
  Token type_token;
  Token identifier;
  uptr<ExprNode<Extra>> initializer;

  VarDeclNode(Token type_token, Token identifier, ExprNode<Extra>* initializer,
              SourceLocation loc)
      : StmtNode<Extra>(loc),
        type_token(type_token),
        identifier(identifier),
        initializer(initializer) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (initializer) initializer->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct IfStmtNode : StmtNode<Extra> {
  uptr<ExprNode<Extra>> condition;
  uptr<StmtNode<Extra>> statement;
  uptr<StmtNode<Extra>> else_stmt;

  IfStmtNode(ExprNode<Extra>* condition, StmtNode<Extra>* statement,
             StmtNode<Extra>* else_stmt, SourceLocation loc)
      : StmtNode<Extra>(loc),
        condition(std::move(condition)),
        statement(std::move(statement)),
        else_stmt(std::move(else_stmt)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (condition) condition->accept(visitor);
    visitor.visit(*this);
    if (statement) statement->accept(visitor);
    if (else_stmt) {
      visitor.before_else(*this);
      else_stmt->accept(visitor);
    }
    visitor.exit(*this);
  }
};

template <typename Extra>
struct WhileStmtNode : StmtNode<Extra> {
  uptr<ExprNode<Extra>> condition;
  uptr<StmtNode<Extra>> statement;

  WhileStmtNode(ExprNode<Extra>* condition, StmtNode<Extra>* statement,
                SourceLocation loc)
      : StmtNode<Extra>(loc),
        condition(std::move(condition)),
        statement(std::move(statement)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (condition) condition->accept(visitor);
    visitor.visit(*this);
    if (statement) statement->accept(visitor);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct ReturnStmtNode : StmtNode<Extra> {
  uptr<ExprNode<Extra>> ret;

  ReturnStmtNode(ExprNode<Extra>* ret, SourceLocation loc)
      : StmtNode<Extra>(loc), ret(std::move(ret)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (ret) ret->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct ExprStmtNode : StmtNode<Extra> {
  uptr<ExprNode<Extra>> expr;

  ExprStmtNode(ExprNode<Extra>* expr, SourceLocation loc)
      : StmtNode<Extra>(loc), expr(expr) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    if (expr) expr->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

/// ============
/// Class-related Nodes
/// ============
template <typename Extra>
struct ClassMemberNode : StmtNode<Extra> {
  ClassMemberNode(SourceLocation loc) : StmtNode<Extra>(loc) {}
};

template <typename Extra>
struct ClassNode : StmtNode<Extra> {
  Token identifier;
  uptr_vector<ClassMemberNode<Extra>> members;

  ClassNode(Token identifier, uptr_vector<ClassMemberNode<Extra>>&& members,
            SourceLocation loc)
      : StmtNode<Extra>(loc),
        identifier(identifier),
        members(std::move(members)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    for (auto& member : members) {
      if (member) member->accept(visitor);
    }
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct FieldDeclNode : ClassMemberNode<Extra> {
  Token access_modifier;
  bool is_static;
  Token type;
  Token identifier;

  FieldDeclNode(Token access_modifier, bool is_static, Token type,
                Token identifier, SourceLocation loc)
      : ClassMemberNode<Extra>(loc),
        access_modifier(access_modifier),
        is_static(is_static),
        type(type),
        identifier(identifier) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct MethodDeclNode : ClassMemberNode<Extra> {
  Token access_modifier;
  bool is_static;
  Token type;
  Token identifier;
  uptr_vector<ParamNode<Extra>> param_list;
  uptr<BlockNode<Extra>> body;

  MethodDeclNode(Token access_modifier, bool is_static, Token type,
                 Token identifier, uptr_vector<ParamNode<Extra>>&& param_list,
                 uptr<BlockNode<Extra>> body, SourceLocation loc)
      : ClassMemberNode<Extra>(loc),
        access_modifier(access_modifier),
        is_static(is_static),
        type(type),
        identifier(identifier),
        param_list(std::move(param_list)),
        body(std::move(body)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    for (auto& param : param_list) {
      if (param) param->accept(visitor);
    }
    if (body) body->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

template <typename Extra>
struct ConstructorDeclNode : ClassMemberNode<Extra> {
  Token identifier;
  uptr_vector<ParamNode<Extra>> param_list;
  uptr<BlockNode<Extra>> body;

  ConstructorDeclNode(Token identifier,
                      uptr_vector<ParamNode<Extra>>&& param_list,
                      uptr<BlockNode<Extra>> body, SourceLocation loc)
      : ClassMemberNode<Extra>(loc),
        identifier(identifier),
        param_list(std::move(param_list)),
        body(std::move(body)) {}

  void accept(ASTVisitor<Extra>& visitor) override {
    visitor.enter(*this);
    for (auto& param : param_list) {
      if (param) param->accept(visitor);
    }
    if (body) body->accept(visitor);
    visitor.visit(*this);
    visitor.exit(*this);
  }
};

struct NodeInfo {
  Symbol* sym;
  TokenType resolved_type = TokenType::TOKEN_UNKNOWN;
  std::string type_name;
};

#endif  // AST_H_
