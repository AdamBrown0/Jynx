#ifndef SYMBOLCOLLECTOR_H_
#define SYMBOLCOLLECTOR_H_

#include "ast.hh"
#include "visitor.hh"

class SymbolCollector : public ASTVisitor {
 public:
  SymbolCollector(CompilerContext& ctx) : ASTVisitor(ctx) {}

  void collect(ProgramNode& program) { collectProgram(program); }

 private:
  void collectStatement(StmtNode& stmt);
  void collectExpression(ExprNode& expr);

  void collectProgram(ProgramNode& node);
  void collectBlock(BlockNode& node);
  void collectMethodDecl(MethodDeclNode& node);
  void collectIfStmt(IfStmtNode& node);
  void collectWhileStmt(WhileStmtNode& node);

  void collectBinaryExpr(BinaryExprNode& node);
  void collectAssignmentExpr(AssignmentExprNode& node);
  void collectMethodCall(MethodCallNode& node);
  void collectArgument(ArgumentNode& node);
  void collectVarDecl(VarDeclNode& node);

  // void report_error(const std::string& message,
  //                   const SourceLocation& location) {
  //   ctx.report_error("Symbol Collector", message, location);
  // }
};

#endif  // SYMBOLCOLLECTOR_H_
