#ifndef METHODTABLE_H_
#define METHODTABLE_H_

#include <functional>
#include <unordered_map>

#include "log.hh"
#include "symbol.hh"

struct MethodKey {
  std::string owner;
  std::string name;

  bool operator==(const MethodKey& other) const {
    return owner == other.owner && name == other.name;
  }
};

struct MethodKeyHash {
  size_t operator()(const MethodKey& key) const {
    return std::hash<std::string>()(key.owner) ^
           (std::hash<std::string>()(key.name) << 1);
  }
};

class MethodTable {
 public:
  bool add_method(FunctionSymbol* method, std::string* error = nullptr) {
    if (!method) return false;

    MethodKey key{method->owner_class, method->name};

    LOG_INFO("owner_class: {}", method->owner_class);

    auto& bucket = methods[key];

    for (const auto& existing : bucket) {
      if (existing->param_types == method->param_types) {
        if (error) *error = "duplicate overload";
        return false;
      }
    }

    bucket.push_back(method);
    return true;
  }

  static std::string make_method_key(const FunctionSymbol& method) {
    std::string key = method.owner_class + "_" + method.name + "_";
    for (size_t i = 0; i < method.fields.size(); ++i) {
      if (i > 0) key += "_";
      key += method.fields[i]->type->to_string();
    }
    return key;
  }

  const FunctionSymbol* find_overload(
      const std::string& owner, const std::string& name,
      const std::vector<const Type*>& param_types) const {
    MethodKey key{owner, name};
    auto it = methods.find(key);
    if (it == methods.end()) return nullptr;

    for (const auto* method : it->second) {
      if (method->param_types == param_types) return method;
    }

    return nullptr;
  }

  const std::vector<FunctionSymbol*> find_all(const std::string& owner,
                                              const std::string& name) const {
    MethodKey key{owner, name};
    auto it = methods.find(key);
    if (it == methods.end()) return {};

    return it->second;
  }

  bool empty() const { return methods.empty(); }

 private:
  std::unordered_map<MethodKey, std::vector<FunctionSymbol*>, MethodKeyHash>
      methods;
};

#endif  // METHODTABLE_H_
