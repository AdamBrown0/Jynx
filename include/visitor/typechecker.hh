#ifndef TYPECHECKER_H_
#define TYPECHECKER_H_

#include "ast.hh"
#include "visitor.hh"

typedef struct TypeInfo {
  TypeNode<NodeInfo> *token_type;
  std::string type_name;
} TypeInfo;

class TypeCheckerVisitor : public ASTVisitor<NodeInfo> {
 public:
  TypeCheckerVisitor(CompilerContext &ctx) : ASTVisitor<NodeInfo>(ctx) {}

  void visit(BinaryExprNode<NodeInfo> &node) override;
  void visit(UnaryExprNode<NodeInfo> &node) override;
  void visit(LiteralExprNode<NodeInfo> &node) override;
  void visit(IdentifierExprNode<NodeInfo> &node) override;
  void visit(AssignmentExprNode<NodeInfo> &node) override;
  void visit(VarDeclNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(MethodCallNode<NodeInfo> &node) override;
  void visit(ExprStmtNode<NodeInfo> &node) override;
  void visit(ReturnStmtNode<NodeInfo> &node) override;
  void visit(ArgumentNode<NodeInfo> &node) override;

  void enter(BlockNode<NodeInfo> &node) override;
  void exit(BlockNode<NodeInfo> &node) override;
  void enter(MethodDeclNode<NodeInfo> &node) override;
  void exit(MethodDeclNode<NodeInfo> &node) override;
  void enter(ConstructorDeclNode<NodeInfo> &node) override;
  void exit(ConstructorDeclNode<NodeInfo> &node) override;
  void enter(ProgramNode<NodeInfo> &node) override;
  void exit(ProgramNode<NodeInfo> &node) override;

 private:
  int current_stack_offset = 0;
  int max_stack_offset = 0;
  std::vector<int> scope_starts;

  int align16(int n) { return (n + 15) & ~15; }

  void add_param_symbol(ParamNode<NodeInfo> &node);


  bool types_compatible(TypeNode<NodeInfo>* declared_type, TypeNode<NodeInfo>* expr_type) {
    if (!declared_type || !expr_type) return false;
    if (declared_type->kind != expr_type->kind) return false;
    if (declared_type->name != expr_type->name) return false;
    return true;
  }


  TypeInfo check_binary_op(TokenType op, const TypeInfo &left, const TypeInfo &right) {
    auto get_builtin_type = [&](const std::string& name) -> TypeNode<NodeInfo>* {
      Symbol* sym = lookup_symbol(name);
      return sym ? sym->type : nullptr;
    };
    switch (op) {
      case TokenType::TOKEN_PLUS:
      case TokenType::TOKEN_MULTIPLY:
        if (left.token_type && left.token_type->name == "string") {
          return {get_builtin_type("string"), "string"};
        }
        // fallthrough
      case TokenType::TOKEN_MINUS:
      case TokenType::TOKEN_DIVIDE:
        if (left.token_type && right.token_type &&
            left.token_type->name == "int" && right.token_type->name == "int") {
          return {get_builtin_type("int"), "int"};
        }
        return {nullptr, ""};
      case TokenType::TOKEN_LT:
      case TokenType::TOKEN_GT:
      case TokenType::TOKEN_LEQ:
      case TokenType::TOKEN_GEQ:
        if (left.token_type && right.token_type &&
            left.token_type->name == "int" && right.token_type->name == "int") {
          return {get_builtin_type("int"), "int"};
        }
        return {nullptr, ""};
      case TokenType::TOKEN_DEQ:
      case TokenType::TOKEN_NEQ:
        if (left.token_type && right.token_type &&
            left.token_type->name == right.token_type->name &&
            (left.token_type->name == "int" || left.token_type->name == "string")) {
          return {get_builtin_type("int"), "int"};
        }
        return {nullptr, ""};
      default:
        return {nullptr, ""};
    }
  }

  TokenType resolve_type(const std::string &name) {
    if (name == "int") return TokenType::TOKEN_INT;
    if (name == "string") return TokenType::TOKEN_STRING;
    if (name == "bool") return TokenType::TOKEN_INT;
    if (name == "char") return TokenType::TOKEN_CHAR;

    Symbol *symbol = lookup_symbol(name);
    if (symbol && symbol->is_class) {
      return TokenType::TOKEN_DATA_TYPE;
    }

    return TokenType::TOKEN_UNKNOWN;
  }

  std::string get_type_name_from_token(TokenType type) {
    switch (type) {
      case TokenType::TOKEN_INT:
        return "int";
      case TokenType::TOKEN_STRING:
        return "string";
      case TokenType::TOKEN_CHAR:
        return "char";
      default:
        return "unknown";
    }
  }
};

#endif  // TYPECHECKER_H_
