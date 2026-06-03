#ifndef POINTER_TYPE_H_
#define POINTER_TYPE_H_

#include <stdexcept>

#include "type.hh"

class PointerType final : public Type {
 public:
  explicit PointerType(const Type* pointee) : pointee(pointee) {
    if (!pointee)
      throw std::runtime_error("Cannot create pointer to null type");
  }

  const Type* get_pointee() const { return pointee; }

  std::string to_string() const override { return pointee->to_string() + "*"; }

  size_t size_in_bytes() const override { return 8; }

  bool equals(const Type& other) const override {
    if (auto* p = dynamic_cast<const PointerType*>(&other)) {
      return pointee->equals(*p->pointee);
    }

    return false;
  }

  bool is_pointer() const override { return true; }

 private:
  const Type* pointee;
};

#endif  // POINTER_TYPE_H_
