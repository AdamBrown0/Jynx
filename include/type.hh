#ifndef TYPE_H_
#define TYPE_H_

#include <string>

class Type {
 public:
  virtual ~Type() = default;

  virtual std::string to_string() const = 0;
  virtual size_t size_in_bytes() const = 0;
  virtual bool equals(const Type& other) const = 0;

  virtual bool is_compatible_with(const Type& other) const {
    return equals(other);
  }

  virtual bool is_primitive() const { return false; }
  virtual bool is_pointer() const { return false; }
  virtual bool is_array() const { return false; }
  virtual bool is_class() const { return false; }
  virtual bool is_void() const { return false; }
};

#endif  // TYPE_H_
