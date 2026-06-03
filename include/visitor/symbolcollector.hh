#ifndef SYMBOLCOLLECTOR_H_
#define SYMBOLCOLLECTOR_H_

#include "ast.hh"
#include "visitor.hh"

class SymbolCollector : public ASTVisitor {
 public:
  SymbolCollector(CompilerContext& ctx) : ASTVisitor(ctx) {}

  void collect(ProgramNode& program);

 private:
  void collectStatement(StmtNode& stmt);
  void collectExpression(ExprNode& expr);

  void collectProgram(ProgramNode& node);
  void collectBlock(BlockNode& node);
  void collectVarDecl(VarDeclNode& node);
  void collectIfStmt(IfStmtNode& node);
  void collectWhileStmt(WhileStmtNode& node);
  void collectReturn(ReturnStmtNode& node);
  void collectExprStmt(ExprStmtNode& node);
  void collectMethodDecl(MethodDeclNode& node);

  void collectBinaryExpr(BinaryExprNode& node);
  void collectUnaryExpr(UnaryExprNode& node);
  void collectLiteralExpr(LiteralExprNode& node);
  void collectIdentifierExpr(IdentifierExprNode& node);
  void collectAssignmentExpr(AssignmentExprNode& node);
  void collectMethodCall(MethodCallNode& node);
  void collectArgument(ArgumentNode& node);

  TokenType resolve_param_type(const std::string& name);
};

#endif  // SYMBOLCOLLECTOR_H_
