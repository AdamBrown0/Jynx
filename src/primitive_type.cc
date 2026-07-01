#include "primitive_type.hh"

#include "context.hh"

std::string PrimitiveType::to_string() const {
  switch (kind) {
    case Kind::Int32:
      return "int";
    case Kind::Bool:
      return "bool";
    case Kind::Void:
      return "void";
    case Kind::Char:
      return "char";
  }
  return "<unknown>";
}

size_t PrimitiveType::size_in_bytes() const {
  switch (kind) {
    case Kind::Int32:
      return 4;
    case Kind::Bool:
      return 1;
    case Kind::Void:
      return 0;
    case Kind::Char:
      return 1;
  }
  return 0;
}

bool PrimitiveType::equals(const Type& other) const {
  if (auto* p = dynamic_cast<const PrimitiveType*>(&other)) {
    if (this->kind == Kind::Int32 && p->kind == Kind::Bool) return true;
    return this->kind == p->kind;
  }
  return false;
}
