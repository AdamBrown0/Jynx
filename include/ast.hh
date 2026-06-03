#ifndef AST_H_
#define AST_H_

#include <memory>
#include <string>
#include <vector>

#include "log.hh"
#include "symbol.hh"
#include "token.hh"
#include "type.hh"

template <typename Extra>
class ASTVisitor;

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
};

/// ============
/// Expressions
/// ============
template <typename Extra>
struct ExprNode : ASTNode<Extra> {
  const Type* result_type = nullptr;

  ExprNode(SourceLocation loc) : ASTNode<Extra>(loc) {}
};

template <typename Extra>
struct BinaryExprNode : ExprNode<Extra> {
  uptr<ExprNode<Extra>> left;
  Token op;
  uptr<ExprNode<Extra>> right;

  BinaryExprNode(ExprNode<Extra>* left, Token op, ExprNode<Extra>* right,
                 SourceLocation loc)
      : ExprNode<Extra>(loc), left(left), op(op), right(right) {}
};

template <typename Extra>
struct UnaryExprNode : ExprNode<Extra> {
  Token op;
  uptr<ExprNode<Extra>> operand;

  UnaryExprNode(Token op, ExprNode<Extra>* operand, SourceLocation loc)
      : ExprNode<Extra>(loc), op(op), operand(operand) {}
};

template <typename Extra>
struct LiteralExprNode : ExprNode<Extra> {
  Token literal_token;

  LiteralExprNode(Token token, SourceLocation loc)
      : ExprNode<Extra>(loc), literal_token(token) {}
};

template <typename Extra>
struct IdentifierExprNode : ExprNode<Extra> {
  Token identifier;

  IdentifierExprNode(Token identifier, SourceLocation loc)
      : ExprNode<Extra>(loc), identifier(identifier) {}
};

template <typename Extra>
struct AssignmentExprNode : ExprNode<Extra> {
  uptr<ExprNode<Extra>> left;
  Token op;
  uptr<ExprNode<Extra>> right;

  AssignmentExprNode(ExprNode<Extra>* left, Token op, ExprNode<Extra>* right,
                     SourceLocation loc)
      : ExprNode<Extra>(loc), left(left), op(op), right(right) {}
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
};

/// ============
/// Supporting Nodes
/// ============
template <typename Extra>
struct ArgumentNode : ASTNode<Extra> {
  uptr<ExprNode<Extra>> expr;

  ArgumentNode(uptr<ExprNode<Extra>> expr, SourceLocation loc)
      : ASTNode<Extra>(loc), expr(std::move(expr)) {}
};

template <typename Extra>
struct ParamNode : ASTNode<Extra> {
  const Type* type = nullptr;
  Token identifier;  // might actually want to replace with something like
                     // vardecl or variable node

  ParamNode(const Type* type, Token identifier, SourceLocation loc)
      : ASTNode<Extra>(loc), type(type), identifier(identifier) {}
};

/// ============
/// Statements
/// ============
template <typename Extra>
struct StmtNode : ASTNode<Extra> {
  StmtNode(SourceLocation loc) : ASTNode<Extra>(loc) {}
};

template <typename Extra>
struct ProgramNode : StmtNode<Extra> {
  uptr_vector<StmtNode<Extra>> children;

  ProgramNode() : StmtNode<Extra>(SourceLocation()) {}
  ProgramNode(uptr_vector<StmtNode<Extra>>&& children)
      : StmtNode<Extra>(SourceLocation()), children(std::move(children)) {}
};

template <typename Extra>
struct BlockNode : StmtNode<Extra> {
  uptr_vector<StmtNode<Extra>> statements;

  BlockNode(uptr_vector<StmtNode<Extra>>&& statements, SourceLocation loc)
      : StmtNode<Extra>(loc), statements(std::move(statements)) {}
};

template <typename Extra>
struct VarDeclNode : StmtNode<Extra> {
  const Type* type = nullptr;
  Token identifier;
  uptr<ExprNode<Extra>> initializer;

  VarDeclNode(const Type* type, Token identifier, ExprNode<Extra>* initializer,
              SourceLocation loc)
      : StmtNode<Extra>(loc),
        type(type),
        identifier(identifier),
        initializer(initializer) {}
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
};

template <typename Extra>
struct ReturnStmtNode : StmtNode<Extra> {
  uptr<ExprNode<Extra>> ret;

  ReturnStmtNode(ExprNode<Extra>* ret, SourceLocation loc)
      : StmtNode<Extra>(loc), ret(std::move(ret)) {}
};

template <typename Extra>
struct ExprStmtNode : StmtNode<Extra> {
  uptr<ExprNode<Extra>> expr;

  ExprStmtNode(ExprNode<Extra>* expr, SourceLocation loc)
      : StmtNode<Extra>(loc), expr(expr) {}
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
};

template <typename Extra>
struct FieldDeclNode : ClassMemberNode<Extra> {
  Token access_modifier;
  bool is_static;
  const Type* type = nullptr;
  Token identifier;

  FieldDeclNode(Token access_modifier, bool is_static, const Type* type,
                Token identifier, SourceLocation loc)
      : ClassMemberNode<Extra>(loc),
        access_modifier(access_modifier),
        is_static(is_static),
        type(type),
        identifier(identifier) {}
};

template <typename Extra>
struct MethodDeclNode : ClassMemberNode<Extra> {
  Token access_modifier;
  bool is_static;
  const Type* type = nullptr;
  Token identifier;
  uptr_vector<ParamNode<Extra>> param_list;
  uptr<BlockNode<Extra>> body;

  MethodDeclNode(Token access_modifier, bool is_static, const Type* type,
                 Token identifier, uptr_vector<ParamNode<Extra>>&& param_list,
                 uptr<BlockNode<Extra>> body, SourceLocation loc)
      : ClassMemberNode<Extra>(loc),
        access_modifier(access_modifier),
        is_static(is_static),
        type(type),
        identifier(identifier),
        param_list(std::move(param_list)),
        body(std::move(body)) {}
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
};

struct NodeInfo {
  std::unique_ptr<Symbol> sym;
  const Type* resolved_type = nullptr;
  std::string type_name;

  int stack_offset = 0;  // for vardecl/ident
  int frame_size = 0;    // for methoddecl/program
  bool has_stack_slot = false;

  size_t param_index = 0;

  std::vector<Symbol> overload_set;
};

#endif  // AST_H_
