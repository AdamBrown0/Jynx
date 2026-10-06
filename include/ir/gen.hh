#ifndef GEN_H_
#define GEN_H_

#include "ast.hh"
#include "context.hh"
#include "ir/basic_block.hh"
#include "ir/function.hh"
#include "ir/module.hh"

class IRGenerator {
 public:
  static constexpr const char* LOG_KIND = "IRGenerator";

  explicit IRGenerator(CompilerContext& ctx) : ctx(ctx) {}

  ir::Module* generateIR(ProgramNode& program);

 private:
  CompilerContext& ctx;

  ir::Module* current_module = nullptr;
  ir::Function* current_function = nullptr;
  ir::BasicBlock* current_block = nullptr;

  std::unordered_map<const Symbol*, ir::Value*> locals;

  unsigned temp_counter = 0;
  unsigned block_counter = 0;

  ir::Value* generateStatement(StmtNode& stmt);
  ir::Value* generateExpression(ExprNode& expr);

  ir::Value* generateExprStmt(ExprStmtNode& node);

  // blocks
  void generateProgram(ProgramNode& node);
  void generateBlock(BlockNode& node);

  // methods
  void generateMethodDecl(MethodDeclNode& node);
  ir::Value* generateMethodCall(MethodCallNode& node);
  void generateParam(ParamNode& node);
  ir::Value* generateArgument(ArgumentNode& node);
  void generateReturnStmt(ReturnStmtNode& node);

  // control flow
  void generateIfStmt(IfStmtNode& node);
  void generateWhileStmt(WhileStmtNode& node);

  // expressions
  ir::Value* generateBinaryExpr(BinaryExprNode& node);
  ir::Value* generateUnaryExpr(UnaryExprNode& node);
  ir::Value* generateLiteralExpr(LiteralExprNode& node);

  // variables
  ir::Value* generateAssignmentExpr(AssignmentExprNode& node);
  ir::Value* generateVarDecl(VarDeclNode& node);
  ir::Value* generateIdentifierExpr(IdentifierExprNode& node);

  // classes
  void generateClassMember(ClassMemberNode& node);
  void generateClass(ClassNode& node);
  void generateFieldDecl(FieldDeclNode& node);
  void generateConstructorDecl(ConstructorDeclNode& node);

  ir::Instruction* emit(ir::Opcode opcode, const Type* type,
                        std::string name = "");

  ir::BasicBlock* createBlock(const std::string& prefix);
  void setBlock(ir::BasicBlock* block);

  ir::Value* makeConstant(const Type* type, const std::string& value);

  std::string nextTemp();
};

#endif  // GEN_H_
