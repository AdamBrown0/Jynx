#ifndef PRIMITIVE_TYPE_H_
#define PRIMITIVE_TYPE_H_

#include "type.hh"

class CompilerContext;

class PrimitiveType final : public Type {
 public:
  enum class Kind { Int32, Bool, Void, Char };

  static const PrimitiveType* Int32();
  static const PrimitiveType* Bool();
  static const PrimitiveType* Void();
  static const PrimitiveType* Char();

  std::string to_string() const override;
  size_t size_in_bytes() const override;
  bool equals(const Type& other) const override;
  bool is_primitive() const override { return true; }
  bool is_void() const override { return kind == Kind::Void; }

 private:
  Kind kind;

  explicit PrimitiveType(Kind k) : kind(k) {}

  friend class CompilerContext;
};

#endif  // PRIMITIVE_TYPE_H_
