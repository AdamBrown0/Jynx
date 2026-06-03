#ifndef ARRAY_TYPE_H_
#define ARRAY_TYPE_H_

#include "type.hh"

class ArrayType final : public Type {
 public:
  ArrayType(const Type* elem, size_t len = 0) : element(elem), length(len) {}

  const Type* get_element() const { return element; }
  size_t get_length() const { return length; }
  bool is_dynamic() const { return length == 0; }

  std::string to_string() const override {
    if (length == 0) return element->to_string() + "[]";
    return element->to_string() + "[" + std::to_string(length) + "]";
  }

  size_t size_in_bytes() const override {
    if (length == 0) return 16;  // ptr + length
    return element->size_in_bytes() * length;
  }

  bool equals(const Type& other) const override {
    if (auto* a = dynamic_cast<const ArrayType*>(&other)) {
      return element->equals(*a->element) && length == a->length;
    }
    return false;
  }

  bool is_array() const override { return true; }

 private:
  const Type* element;
  size_t length;
};

#endif  // ARRAY_TYPE_H_
