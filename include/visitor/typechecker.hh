#ifndef TYPECHECKER_H_
#define TYPECHECKER_H_

#include "ast.hh"
#include "visitor.hh"

class TypeChecker : public ASTVisitor {
 public:
  TypeChecker(CompilerContext& ctx) : ASTVisitor(ctx) {}

  void check(ProgramNode& program) { checkProgram(program); }

 private:
  FunctionSymbol* current_function = nullptr;

  void checkStatement(StmtNode& stmt);
  const Type* checkExpression(ExprNode& expr);

  void checkProgram(ProgramNode& node);
  void checkBlock(BlockNode& node);
  const Type* checkVarDecl(VarDeclNode& node);
  void checkIfStmt(IfStmtNode& node);
  void checkWhileStmt(WhileStmtNode& node);
  void checkReturn(ReturnStmtNode& node);
  void checkExprStmt(ExprStmtNode& node);
  void checkMethodDecl(MethodDeclNode& node);

  const Type* checkBinaryExpr(BinaryExprNode& node);
  const Type* checkUnaryExpr(UnaryExprNode& node);
  const Type* checkLiteralExpr(LiteralExprNode& node);
  const Type* checkIdentifierExpr(IdentifierExprNode& node);
  const Type* checkAssignmentExpr(AssignmentExprNode& node);
  const Type* checkMethodCall(MethodCallNode& node);
  const Type* checkArgument(ArgumentNode& node);

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
      case TokenType::TOKEN_EQUALS:
        return left.equals(right) ? ctx.get_bool_type() : ctx.get_void_type();
      default:
        return ctx.get_void_type();
    }
  }
};

#endif  // TYPECHECKER_H_
