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
  NameResolver(CompilerContext &ctx) : ASTVisitor<NodeInfo>(ctx) {}

  void enter(BlockNode<NodeInfo> &node) override;
  void exit(BlockNode<NodeInfo> &node) override;
  void enter(ProgramNode<NodeInfo> &node) override;
  void exit(ProgramNode<NodeInfo> &node) override;
  void enter(MethodDeclNode<NodeInfo> &node) override;
  void exit(MethodDeclNode<NodeInfo> &node) override;
  void enter(ConstructorDeclNode<NodeInfo> &node) override;
  void exit(ConstructorDeclNode<NodeInfo> &node) override;
  void enter(ClassNode<NodeInfo> &node) override;
  void exit(ClassNode<NodeInfo> &node) override;

  void visit(VarDeclNode<NodeInfo> &node) override;
  void visit(ParamNode<NodeInfo> &node) override;
  void visit(ArgumentNode<NodeInfo> &node) override;
  void visit(MethodDeclNode<NodeInfo> &node) override;
  void visit(IdentifierExprNode<NodeInfo> &node) override;
  void visit(ClassNode<NodeInfo> &node) override;
  void visit(MethodCallNode<NodeInfo> &node) override;
  void visit(AssignmentExprNode<NodeInfo> &node) override;

 private:
  const std::vector<Symbol> *find_method_overloads(const std::string &owner,
                                                   const std::string &name);
};

#endif  // NAMERESOLVER_H_
