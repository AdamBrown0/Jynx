#ifndef NAMERESOLVER_H_
#define NAMERESOLVER_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "ast.hh"
#include "methodtable.hh"
#include "visitor/visitor.hh"

class NameResolver : public ASTVisitor<NodeInfo> {
 public:
  NameResolver(CompilerContext& ctx) : ASTVisitor<NodeInfo>(ctx) {}

  void resolve(ProgramNode<NodeInfo>& program);

 private:
  void resolveStatement(StmtNode<NodeInfo>& stmt);
  void resolveExpression(ExprNode<NodeInfo>& expr);

  void resolveProgram(ProgramNode<NodeInfo>& node);
  void resolveBlock(BlockNode<NodeInfo>& node);
  void resolveVarDecl(VarDeclNode<NodeInfo>& node);
  void resolveIfStmt(IfStmtNode<NodeInfo>& node);
  void resolveWhileStmt(WhileStmtNode<NodeInfo>& node);
  void resolveReturn(ReturnStmtNode<NodeInfo>& node);
  void resolveExprStmt(ExprStmtNode<NodeInfo>& node);
  void resolveMethodDecl(MethodDeclNode<NodeInfo>& node);

  void resolveBinaryExpr(BinaryExprNode<NodeInfo>& node);
  void resolveUnaryExpr(UnaryExprNode<NodeInfo>& node);
  void resolveLiteralExpr(LiteralExprNode<NodeInfo>& node);
  void resolveIdentifierExpr(IdentifierExprNode<NodeInfo>& node);
  void resolveAssignmentExpr(AssignmentExprNode<NodeInfo>& node);
  void resolveMethodCall(MethodCallNode<NodeInfo>& node);
  void resolveArgument(ArgumentNode<NodeInfo>& node);

  const std::vector<Symbol>* find_method_overloads(const std::string& owner,
                                                   const std::string& name);
};

#endif  // NAMERESOLVER_H_
