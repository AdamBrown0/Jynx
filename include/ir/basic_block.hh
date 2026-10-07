#ifndef BASIC_BLOCK_H_
#define BASIC_BLOCK_H_

#include <memory>
#include <vector>

#include "ir/instruction.hh"

namespace ir {

class BasicBlock : public Value {
 public:
  std::vector<std::unique_ptr<Instruction>> instructions;
  std::vector<BasicBlock*> predecessors;
  std::vector<BasicBlock*> successors;

  BasicBlock(std::string label)
      : Value(CompilerContext::instance().get_void_type(), std::move(label)) {}

  ~BasicBlock() {
    while (!instructions.empty()) {
      instructions.pop_back();  // destroys the unique_ptr
    }
  }

  Instruction* appendInstruction(std::unique_ptr<Instruction> instruction) {
    instruction->parent = this;
    instructions.push_back(std::move(instruction));
    return instructions.back().get();
  }

  static void addEdge(BasicBlock* from, BasicBlock* to) {
    from->successors.push_back(to);
    to->predecessors.push_back(from);
  }
};

}  // namespace ir

#endif  // BASIC_BLOCK_H_
