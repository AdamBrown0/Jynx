#ifndef SYMBOLCOLLECTOR_H_
#define SYMBOLCOLLECTOR_H_

#include "ast.hh"
#include "visitor.hh"

class SymbolCollector : public ASTVisitor<NodeInfo> {
 public:
  SymbolCollector(CompilerContext& ctx) : ASTVisitor<NodeInfo>(ctx) {}

  void collect(ProgramNode<NodeInfo>& program);

 private:
  void collectStatement(StmtNode<NodeInfo>& stmt);
  void collectExpression(ExprNode<NodeInfo>& expr);

  void collectProgram(ProgramNode<NodeInfo>& node);
  void collectBlock(BlockNode<NodeInfo>& node);
  void collectVarDecl(VarDeclNode<NodeInfo>& node);
  void collectIfStmt(IfStmtNode<NodeInfo>& node);
  void collectWhileStmt(WhileStmtNode<NodeInfo>& node);
  void collectReturn(ReturnStmtNode<NodeInfo>& node);
  void collectExprStmt(ExprStmtNode<NodeInfo>& node);
  void collectMethodDecl(MethodDeclNode<NodeInfo>& node);

  void collectBinaryExpr(BinaryExprNode<NodeInfo>& node);
  void collectUnaryExpr(UnaryExprNode<NodeInfo>& node);
  void collectLiteralExpr(LiteralExprNode<NodeInfo>& node);
  void collectIdentifierExpr(IdentifierExprNode<NodeInfo>& node);
  void collectAssignmentExpr(AssignmentExprNode<NodeInfo>& node);
  void collectMethodCall(MethodCallNode<NodeInfo>& node);
  void collectArgument(ArgumentNode<NodeInfo>& node);

  TokenType resolve_param_type(const std::string& name);
};

#endif  // SYMBOLCOLLECTOR_H_
