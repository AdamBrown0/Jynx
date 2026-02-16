#ifndef METHODTABLE_H_
#define METHODTABLE_H_

#include <functional>
#include <unordered_map>

#include "log.hh"
#include "symbol.hh"
#include "token.hh"

struct MethodKey {
  std::string owner;
  std::string name;

  bool operator==(const MethodKey &other) const {
    return owner == other.owner && name == other.name;
  }
};

struct MethodKeyHash {
  size_t operator()(const MethodKey &key) const {
    return std::hash<std::string>()(key.owner) ^
           (std::hash<std::string>()(key.name) << 1);
  }
};

class MethodTable {
 public:
  bool add_method(const Symbol &method, std::string *error = nullptr) {
    MethodKey key{method.owner_class, method.name};
    auto &bucket = methods[key];

    for (const auto &existing : bucket) {
      if (existing.param_types == method.param_types) {
        if (error) *error = "duplicate overload";
        return false;
      }
    }

    bucket.push_back(method);
    return true;
  }

  static std::string make_method_key(const Symbol &method) {
    std::string key = method.owner_class + "_" + method.name + "_";
    for (size_t i = 0; i < method.param_types.size(); ++i) {
      if (i > 0) key += "_";
      key += token_type_to_string(method.param_types[i]);
    }
    return key;
  }

  const Symbol *find_overload(const std::string &owner, const std::string &name,
                              const std::vector<TokenType> &param_types) const {
    MethodKey key{owner, name};
    auto it = methods.find(key);
    if (it == methods.end()) return nullptr;

    for (const auto &method : it->second) {
      if (method.param_types == param_types) return &method;
    }
    return nullptr;
  }

  const std::vector<Symbol> *find_all(const std::string &owner,
                                      const std::string &name) const {
    MethodKey key{owner, name};
    auto it = methods.find(key);
    if (it == methods.end()) return nullptr;
    return &it->second;
  }

  bool empty() const { return methods.empty(); }

 private:
  std::unordered_map<MethodKey, std::vector<Symbol>, MethodKeyHash> methods;
};

#endif  // METHODTABLE_H_
