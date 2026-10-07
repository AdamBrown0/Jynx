#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <cstdint>
#include <vector>
#include <assert.h>

#include "context.hh"
#include "type.hh"

namespace ir {

enum class Opcode {
  Alloca,
  Load,
  Store,
  Add,
  Sub,
  Mul,
  Div,

  Br,
  CondBr,
  Ret,

  Phi,
  Upsilon
};

class User;
class BasicBlock;

class Value {
 public:
  Value(const Type* type, std::string name = "")
      : type(type), name(std::move(name)) {}

  ~Value() {
  	assert(uses.empty() && "destroying Value that still has users");
  }

  void addUse(User* user) { uses.push_back(user); }
  void removeUse(User* user) {
    uses.erase(std::remove(uses.begin(), uses.end(), user), uses.end());
  }

  const Type* type;
  std::string name;
  std::vector<User*> uses;
};

struct Use {
  Value* value;
  User* user;

  Use(Value* value, User* user) : value(value), user(user) {
    if (value) value->addUse(user);
  }

  ~Use() {
    if (value) value->removeUse(user);
  }

  void set(Value* newValue) {
    if (value) value->removeUse(user);
    value = newValue;
    if (value) value->addUse(user);
  }
};

class User : public Value {
 protected:
  std::vector<Use> operands;

 public:
  User(const Type* type, std::string name = "")
      : Value(type, std::move(name)) {}

  Value* getOperand(size_t index) const { return operands[index].value; }
  void setOperand(size_t index, Value* value) { operands[index].set(value); }
  size_t getNumOperands() const { return operands.size(); }

  void addOperand(Value* value) { operands.emplace_back(value, this); }
};

class Instruction : public User {
 public:
  Opcode op;
  BasicBlock* parent;

  Instruction(Opcode op, const Type* type, BasicBlock* parent = nullptr,
              std::string name = "")
      : User(type, std::move(name)), op(op), parent(parent) {}
};

class UpsilonInst : public Instruction {
 public:
  class PhiInst* targetPhi;

  UpsilonInst(Value* sourceValue, PhiInst* targetPhi,
              BasicBlock* parent = nullptr)
      : Instruction(Opcode::Upsilon,
                    CompilerContext::instance().get_void_type(), parent),
        targetPhi(targetPhi) {
    addOperand(sourceValue);
  }

  Value* getValue() const { return getOperand(0); }
};

class PhiInst : public Instruction {
 public:
  PhiInst(const Type* type, std::string name = "", BasicBlock* parent = nullptr)
      : Instruction(Opcode::Phi, type, parent, std::move(name)) {}
};

}  // namespace ir

#endif  // INSTRUCTION_H_
