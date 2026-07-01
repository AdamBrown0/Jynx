#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "array_type.hh"
#include "methodtable.hh"
#include "pointer_type.hh"
#include "primitive_type.hh"
#include "scope.hh"
#include "trie.hh"
#include "type.hh"

class CompilerContext {
 public:
  CompilerContext();

  std::unordered_map<std::string, Symbol> symbol_table;
  MethodTable method_table;
  KeywordTrie keywords;

  const Type* get_int32_type();
  const Type* get_bool_type();
  const Type* get_void_type();
  const Type* get_char_type();

  const Type* make_pointer_type(const Type* pointee);
  const Type* make_array_type(const Type* element, size_t length = 0);
  const Type* make_class_type(const std::string& class_name);

  void report_error(const std::string& error_kind, const std::string& message,
                    SourceLocation location);
  void report_warning(const std::string& warning_kind,
                      const std::string& message,
                      const SourceLocation& location);

  void push_scope();
  void pop_scope();
  Scope* get_current_scope() const { return current_scope; }
  void set_current_scope(Scope* scope) { current_scope = scope; }

  Symbol* declare(const std::string& name, const Type* type,
                  SourceLocation loc);
  VariableSymbol* declare(const std::string& name, const Type* type,
                          bool is_mutable, SourceLocation loc);
  FunctionSymbol* declare(const std::string& name, const Type* type,
                          const std::vector<const Type*> param_types,
                          SourceLocation loc);

  Symbol* lookup(const std::string& name, bool walkParent = true);
  Symbol* lookup(const std::string& name, Scope* startingScope,
                 bool walkParent = true);

 private:
  std::vector<std::string> errors;

  std::vector<std::unique_ptr<Type>> type_storage;
  std::vector<std::unique_ptr<Scope>> scope_storage;

  Scope* current_scope = nullptr;

  const PrimitiveType* int32_type = nullptr;
  const PrimitiveType* bool_type = nullptr;
  const PrimitiveType* void_type = nullptr;
  const PrimitiveType* char_type = nullptr;

  std::unordered_map<const Type*, const PointerType*> pointer_cache;
  std::unordered_map<std::string, const ArrayType*> array_cache;
  // std::unordered_map<std::string, const ClassType*> class_cache;

  template <typename T, typename... Args>
  const T* create_type(Args&&... args) {
    auto ptr = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    const T* raw = ptr.get();
    type_storage.push_back(std::move(ptr));
    return raw;
  }
};

#endif  // CONTEXT_H_
