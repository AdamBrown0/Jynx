#ifndef TYPECHECKER_H_
#define TYPECHECKER_H_

#include "ast.hh"
#include "visitor.hh"

class TypeChecker : public ASTVisitor<NodeInfo> {
 public:
  TypeChecker(CompilerContext& ctx) : ASTVisitor<NodeInfo>(ctx) {}

  void check(ProgramNode<NodeInfo>& program) { checkProgram(program); }

 private:
  void checkStatement(StmtNode<NodeInfo>& stmt);
  const Type* checkExpression(ExprNode<NodeInfo>& expr);

  void checkProgram(ProgramNode<NodeInfo>& node);
  void checkBlock(BlockNode<NodeInfo>& node);
  void checkVarDecl(VarDeclNode<NodeInfo>& node);
  void checkIfStmt(IfStmtNode<NodeInfo>& node);
  void checkWhileStmt(WhileStmtNode<NodeInfo>& node);
  void checkReturn(ReturnStmtNode<NodeInfo>& node);
  void checkExprStmt(ExprStmtNode<NodeInfo>& node);
  void checkMethodDecl(MethodDeclNode<NodeInfo>& node);

  const Type* checkBinaryExpr(BinaryExprNode<NodeInfo>& node);
  const Type* checkUnaryExpr(UnaryExprNode<NodeInfo>& node);
  const Type* checkLiteralExpr(LiteralExprNode<NodeInfo>& node);
  const Type* checkIdentifierExpr(IdentifierExprNode<NodeInfo>& node);
  const Type* checkAssignmentExpr(AssignmentExprNode<NodeInfo>& node);
  const Type* checkMethodCall(MethodCallNode<NodeInfo>& node);
  const Type* checkArgument(ArgumentNode<NodeInfo>& node);

  const Type* check_binary_op(TokenType op, const Type& left,
                              const Type& right) {
    switch (op) {
      case TokenType::TOKEN_PLUS:
      case TokenType::TOKEN_MULTIPLY:
      // "str" * 2 == 2 * "str" == "strstr"
      case TokenType::TOKEN_MINUS:
      case TokenType::TOKEN_DIVIDE:
        if (left.equals(*ctx.get_int32_type()) &&
            right.equals(*ctx.get_int32_type()))
          return ctx.get_int32_type();
        return ctx.get_void_type();
      case TokenType::TOKEN_LT:
      case TokenType::TOKEN_GT:
      case TokenType::TOKEN_LEQ:
      case TokenType::TOKEN_GEQ:
        if (left.equals(*ctx.get_int32_type()) &&
            right.equals(*ctx.get_int32_type()))
          return ctx.get_bool_type();
        return ctx.get_void_type();
      case TokenType::TOKEN_DEQ:
      case TokenType::TOKEN_NEQ:
        if (left.equals(*ctx.get_int32_type()) &&
            right.equals(*ctx.get_int32_type()))
          return ctx.get_bool_type();
      default:
        return ctx.get_void_type();
    }
  }

  // TokenType resolve_type(const std::string& name) {
  //   if (name == "int") return TokenType::TOKEN_INT;
  //   if (name == "string") return TokenType::TOKEN_STRING;
  //   if (name == "bool") return TokenType::TOKEN_INT;
  //   if (name == "char") return TokenType::TOKEN_CHAR;

  //   Symbol* symbol = lookup_symbol(name);
  //   if (symbol && symbol->is_class) {
  //     return TokenType::TOKEN_DATA_TYPE;
  //   }

  //   return TokenType::TOKEN_UNKNOWN;
  // }

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
