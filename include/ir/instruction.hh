#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <assert.h>

#include <cstdint>
#include <iostream>
#include <vector>

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
  Call,

  Phi,
  Upsilon
};

class User;
class Value;
class BasicBlock;
class Instruction;

class Use {
 public:
  Value* value = nullptr;
  User* user = nullptr;

  Use* prev = nullptr;
  Use* next = nullptr;

  Use() = default;
  Use(Value* v, User* u);
  ~Use();

  Use(const Use&) = delete;
  Use& operator=(const Use&) = delete;
  Use(Use&&) = delete;
  Use& operator=(Use&&) = delete;

  void set(Value* newValue);
};

class Value {
 public:
  Value(const Type* type, std::string name = "")
      : type(type), name(std::move(name)) {}

  // ~Value() {
  //   assert(use_list == nullptr && "destroying Value that still has users");
  // }
  ~Value() {
    if (use_list) {
      std::cerr << "!!! still has users: name=\"" << name << "\" ptr=" << this
                << "\n";
      for (Use* u = use_list; u; u = u->next)
        std::cerr << "    used by " << u->user << "\n";
    }
    assert(use_list == nullptr);
  }
  void addUse(Use* u) {
    u->next = use_list;
    u->prev = nullptr;
    if (use_list) use_list->prev = u;
    use_list = u;
  }
  void removeUse(Use* u) {
    if (u->prev)
      u->prev->next = u->next;
    else
      use_list = u->next;

    if (u->next) u->next->prev = u->prev;

    u->prev = u->next = nullptr;
  }

  bool use_empty() const { return use_list == nullptr; }

  const Type* type;
  std::string name;

  Use* use_list = nullptr;
};

inline Use::Use(Value* v, User* u) : value(v), user(u) {
  if (value) value->addUse(this);
}

inline Use::~Use() {
  if (value) value->removeUse(this);
}

inline void Use::set(Value* newValue) {
  if (value) value->removeUse(this);
  value = newValue;
  if (value) value->addUse(this);
}

class User : public Value {
 protected:
  std::vector<Use*> operands;

 public:
  User(const Type* type, std::string name = "")
      : Value(type, std::move(name)) {}

  // ~User() {
  //   std::cerr << "~User " << this << "  #operands" << operands.size() <<
  //   "\n"; for (Use* u : operands) {
  //     delete u;
  //   }
  //   operands.clear();
  // }

  ~User() {
    for (Use* u : operands) delete u;
  }

  Value* getOperand(size_t index) const {
    assert(index < operands.size());
    return operands[index]->value;
  }

  void setOperand(size_t index, Value* value) {
    assert(index < operands.size());
    operands[index]->set(value);
  }

  size_t getNumOperands() const { return operands.size(); }

  void addOperand(Value* value) { operands.push_back(new Use(value, this)); }
};

class Instruction : public User {
 public:
  Opcode op;
  BasicBlock* parent = nullptr;

  Instruction(Opcode op, const Type* type, BasicBlock* parent = nullptr,
              std::string name = "")
      : User(type, std::move(name)), op(op), parent(parent) {}
};

class PhiInst : public Instruction {
 public:
  PhiInst(const Type* type, std::string name = "", BasicBlock* parent = nullptr)
      : Instruction(Opcode::Phi, type, parent, std::move(name)) {}
};

class UpsilonInst : public Instruction {
 public:
  PhiInst* targetPhi;

  UpsilonInst(Value* sourceValue, PhiInst* targetPhi,
              BasicBlock* parent = nullptr)
      : Instruction(Opcode::Upsilon,
                    CompilerContext::instance().get_void_type(), parent),
        targetPhi(targetPhi) {
    addOperand(sourceValue);
  }

  Value* getValue() const { return getOperand(0); }
};

}  // namespace ir

#endif  // INSTRUCTION_H_
