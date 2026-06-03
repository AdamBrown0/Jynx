#include "context.hh"

CompilerContext::CompilerContext() {
  int32_type = create_type<PrimitiveType>(PrimitiveType::Kind::Int32);
  bool_type = create_type<PrimitiveType>(PrimitiveType::Kind::Bool);
  void_type = create_type<PrimitiveType>(PrimitiveType::Kind::Void);
  char_type = create_type<PrimitiveType>(PrimitiveType::Kind::Char);
}

const Type* CompilerContext::get_int32_type() { return int32_type; }
const Type* CompilerContext::get_bool_type() { return bool_type; }
const Type* CompilerContext::get_void_type() { return void_type; }
const Type* CompilerContext::get_char_type() { return char_type; }

const Type* CompilerContext::make_pointer_type(const Type* pointee) {
  if (!pointee) return nullptr;

  auto it = pointer_cache.find(pointee);
  if (it != pointer_cache.end()) return it->second;

  const PointerType* t = create_type<PointerType>(pointee);
  pointer_cache[pointee] = t;
  return t;
}

const Type* CompilerContext::make_array_type(const Type* element,
                                             size_t length) {
  if (!element) return nullptr;

  std::stringstream ss;
  ss << element << ":" << length;
  std::string key = ss.str();

  auto it = array_cache.find(key);
  if (it != array_cache.end()) return it->second;

  const ArrayType* t = create_type<ArrayType>(element, length);
  array_cache[key] = t;
  return t;
}

// const Type* CompilerContext::make_class_type(const std::string& class_name) {
//   if (class_name.empty()) return nullptr;

//   auto it = class_cache.find(class_name);
//   if (it != class_cache.end()) return it->second;

//   const ClassType* t = create_type<ClassType>(class_name);
//   class_cache[class_name] = t;
//   return t;
// }
