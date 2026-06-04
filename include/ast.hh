#ifndef AST_H_
#define AST_H_

#include <memory>
#include <string>
#include <vector>

#include "log.hh"
#include "symbol.hh"
#include "token.hh"
#include "type.hh"

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T>
using uptr_vector = std::vector<uptr<T>>;

// Forward declarations
struct ExprNode;
struct StmtNode;
struct ClassMemberNode;
struct ParamNode;
struct BlockNode;
struct ElseStmtNode;

struct SemanticInfo {
  const Type* declared_type = nullptr;
  Symbol* symbol = nullptr;
  bool is_mutable = true;
};

struct CodegenInfo {
  int stack_offset = -1;
  int temp_id = -1;  // for expression temps
  bool is_lvalue = false;
};

struct ASTNode {
  SourceLocation location;

  SemanticInfo semantic;
  CodegenInfo codegen;

  ASTNode(SourceLocation loc) : location(loc) {}
  virtual ~ASTNode() = default;
};

/// ============
/// Expressions
/// ============
struct ExprNode : ASTNode {
  const Type* result_type = nullptr;

  ExprNode(SourceLocation loc) : ASTNode(loc) {}
};

struct BinaryExprNode : ExprNode {
  uptr<ExprNode> left;
  Token op;
  uptr<ExprNode> right;

  BinaryExprNode(ExprNode* left, Token op, ExprNode* right, SourceLocation loc)
      : ExprNode(loc), left(left), op(op), right(right) {}
};

struct UnaryExprNode : ExprNode {
  Token op;
  uptr<ExprNode> operand;

  UnaryExprNode(Token op, ExprNode* operand, SourceLocation loc)
      : ExprNode(loc), op(op), operand(operand) {}
};

struct LiteralExprNode : ExprNode {
  Token literal_token;

  LiteralExprNode(Token token, SourceLocation loc)
      : ExprNode(loc), literal_token(token) {}
};

struct IdentifierExprNode : ExprNode {
  Token identifier;

  IdentifierExprNode(Token identifier, SourceLocation loc)
      : ExprNode(loc), identifier(identifier) {}
};

struct AssignmentExprNode : ExprNode {
  uptr<ExprNode> left;
  Token op;
  uptr<ExprNode> right;

  AssignmentExprNode(ExprNode* left, Token op, ExprNode* right,
                     SourceLocation loc)
      : ExprNode(loc), left(left), op(op), right(right) {}
};

struct ArgumentNode : ExprNode {
  uptr<ExprNode> expr;

  ArgumentNode(uptr<ExprNode> expr, SourceLocation loc)
      : ExprNode(loc), expr(std::move(expr)) {}
};

struct MethodCallNode : ExprNode {
  uptr<ExprNode> expr;
  Token identifier;
  uptr_vector<ArgumentNode> arg_list;

  MethodCallNode(uptr<ExprNode>&& expr, Token identifier,
                 uptr_vector<ArgumentNode>&& arg_list, SourceLocation loc)
      : ExprNode(loc),
        expr(std::move(expr)),
        identifier(identifier),
        arg_list(std::move(arg_list)) {}
};

/// ============
/// Supporting Nodes
/// ============

struct ParamNode : ASTNode {
  const Type* declared_type = nullptr;
  Token identifier;

  ParamNode(const Type* type, Token identifier, SourceLocation loc)
      : ASTNode(loc), declared_type(type), identifier(identifier) {}
};

/// ============
/// Statements
/// ============

struct StmtNode : ASTNode {
  StmtNode(SourceLocation loc) : ASTNode(loc) {}
};

struct ProgramNode : StmtNode {
  uptr_vector<StmtNode> children;

  ProgramNode() : StmtNode(SourceLocation()) {}
  ProgramNode(uptr_vector<StmtNode>&& children)
      : StmtNode(SourceLocation()), children(std::move(children)) {}
};

struct BlockNode : StmtNode {
  uptr_vector<StmtNode> statements;

  BlockNode(uptr_vector<StmtNode>&& statements, SourceLocation loc)
      : StmtNode(loc), statements(std::move(statements)) {}
};

struct VarDeclNode : ExprNode {
  const Type* declared_type = nullptr;
  Token identifier;
  uptr<ExprNode> initializer;

  VarDeclNode(const Type* type, Token identifier, ExprNode* initializer,
              SourceLocation loc)
      : ExprNode(loc),
        declared_type(type),
        identifier(identifier),
        initializer(initializer) {}
};

struct IfStmtNode : StmtNode {
  uptr<ExprNode> condition;
  uptr<StmtNode> statement;
  uptr<StmtNode> else_stmt;

  IfStmtNode(ExprNode* condition, StmtNode* statement, StmtNode* else_stmt,
             SourceLocation loc)
      : StmtNode(loc),
        condition(std::move(condition)),
        statement(std::move(statement)),
        else_stmt(std::move(else_stmt)) {}
};

struct WhileStmtNode : StmtNode {
  uptr<ExprNode> condition;
  uptr<StmtNode> statement;

  WhileStmtNode(ExprNode* condition, StmtNode* statement, SourceLocation loc)
      : StmtNode(loc),
        condition(std::move(condition)),
        statement(std::move(statement)) {}
};

struct ReturnStmtNode : StmtNode {
  uptr<ExprNode> ret;

  ReturnStmtNode(ExprNode* ret, SourceLocation loc)
      : StmtNode(loc), ret(std::move(ret)) {}
};

struct ExprStmtNode : StmtNode {
  uptr<ExprNode> expr;

  ExprStmtNode(ExprNode* expr, SourceLocation loc)
      : StmtNode(loc), expr(expr) {}
};

/// ============
/// Class-related Nodes
/// ============

struct ClassMemberNode : StmtNode {
  ClassMemberNode(SourceLocation loc) : StmtNode(loc) {}
};

struct ClassNode : StmtNode {
  Token identifier;
  uptr_vector<ClassMemberNode> members;

  ClassNode(Token identifier, uptr_vector<ClassMemberNode>&& members,
            SourceLocation loc)
      : StmtNode(loc), identifier(identifier), members(std::move(members)) {}
};

struct FieldDeclNode : ClassMemberNode {
  Token access_modifier;
  bool is_static;
  const Type* declared_type = nullptr;
  Token identifier;

  FieldDeclNode(Token access_modifier, bool is_static, const Type* type,
                Token identifier, SourceLocation loc)
      : ClassMemberNode(loc),
        access_modifier(access_modifier),
        is_static(is_static),
        declared_type(type),
        identifier(identifier) {}
};

struct MethodDeclNode : ClassMemberNode {
  Token access_modifier;
  bool is_static;
  const Type* declared_type = nullptr;
  Token identifier;
  uptr_vector<ParamNode> param_list;
  uptr<BlockNode> body;

  MethodDeclNode(Token access_modifier, bool is_static, const Type* type,
                 Token identifier, uptr_vector<ParamNode>&& param_list,
                 uptr<BlockNode> body, SourceLocation loc)
      : ClassMemberNode(loc),
        access_modifier(access_modifier),
        is_static(is_static),
        declared_type(type),
        identifier(identifier),
        param_list(std::move(param_list)),
        body(std::move(body)) {}
};

struct ConstructorDeclNode : ClassMemberNode {
  Token identifier;
  uptr_vector<ParamNode> param_list;
  uptr<BlockNode> body;

  ConstructorDeclNode(Token identifier, uptr_vector<ParamNode>&& param_list,
                      uptr<BlockNode> body, SourceLocation loc)
      : ClassMemberNode(loc),
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
