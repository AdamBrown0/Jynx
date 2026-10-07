#ifndef PRINTER_H_
#define PRINTER_H_

#include <iostream>
#include <sstream>
#include <string>

#include "ir/function.hh"
#include "type.hh"

class IRPrinter {
 public:
  static void print(const ir::Function& func, std::ostream& os = std::cout) {
    os << "define " << func.returnType->to_string() << " @" << func.name
       << "() {\n";
    for (const auto& block : func.blocks) {
      printBlock(*block, os);
    }
    os << "}\n";
  }

 private:
  static std::string opcodeToString(ir::Opcode op) {
    switch (op) {
      case ir::Opcode::Alloca:
        return "alloca";
      case ir::Opcode::Load:
        return "load";
      case ir::Opcode::Store:
        return "store";
      case ir::Opcode::Add:
        return "add";
      case ir::Opcode::Sub:
        return "sub";
      case ir::Opcode::Mul:
        return "mul";
      case ir::Opcode::Div:
        return "div";
      case ir::Opcode::Br:
        return "br";
      case ir::Opcode::CondBr:
        return "condbr";
      case ir::Opcode::Ret:
        return "ret";
      case ir::Opcode::Call:
        return "call";
      case ir::Opcode::Phi:
        return "phi";
      case ir::Opcode::Upsilon:
        return "upsilon";
    }
    return "unknown";
  }

  static std::string formatValueRef(const ir::Value* val) {
    if (!val) return "null";
    if (val->name.empty()) return "%tmp";
    // Check if it's a literal number/constant proxy or a register name
    if (std::isdigit(val->name[0]) || val->name[0] == '-') {
      return val->name;
    }
    return "%" + val->name;
  }

  static void printBlock(const ir::BasicBlock& block, std::ostream& os) {
    os << block.name << ":";

    // Print predecessors as a comment for easier debugging
    if (!block.predecessors.empty()) {
      os << " \t; preds: ";
      for (size_t i = 0; i < block.predecessors.size(); ++i) {
        os << "%" << block.predecessors[i]->name;
        if (i + 1 < block.predecessors.size()) os << ", ";
      }
    }
    os << "\n";

    for (const auto& inst : block.instructions) {
      os << "  ";
      printInstruction(*inst, os);
      os << "\n";
    }
  }

  static void printInstruction(const ir::Instruction& inst, std::ostream& os) {
    // 1. Special Handling for Upsilon Insts
    if (inst.op == ir::Opcode::Upsilon) {
      const auto* upsilon = static_cast<const ir::UpsilonInst*>(&inst);
      os << "upsilon " << formatValueRef(upsilon->getValue()) << ", "
         << formatValueRef(upsilon->targetPhi);
      return;
    }

    // 2. Non-void instructions get a LHS variable destination
    if (inst.op != ir::Opcode::Ret &&
        inst.type != CompilerContext::instance().get_void_type()) {
      os << formatValueRef(&inst) << " = ";
    }

    // 3. Print Opcode
    os << opcodeToString(inst.op);

    // 4. Special Handling for Phi Insts
    if (inst.op == ir::Opcode::Phi) {
      os << " " << inst.type->to_string();
      return;
    }

    // 5. Standard Instructions
    if (inst.type != CompilerContext::instance().get_void_type()) {
      os << " " << inst.type->to_string();
    }

    for (size_t i = 0; i < inst.getNumOperands(); ++i) {
      os << (i == 0 ? " " : ", ") << formatValueRef(inst.getOperand(i));
    }
  }
};
#endif  // PRINTER_H_
