#ifndef TOKEN_UTILS_H_
#define TOKEN_UTILS_H_

#include "ast.hh"
#include "log.hh"
#include "token.hh"

namespace TokenUtils {

inline const char* token_type_to_string(TokenType type) {
  switch (type) {
#define X(name)         \
  case TokenType::name: \
    return #name;
    TOKEN_LIST
#undef X
    default:
      return "UNKNOWN";
  }
}

// inline const std::string type_to_string(TypeNode<NodeInfo>* type) {
//   if (!type) return "void";

//   switch (type->kind) {
//     case TypeNode<NodeInfo>::Primitive:
//       return type->name;
//     case TypeNode<NodeInfo>::Array:
//       return type_to_string(type->element.get()) + "[]";
//     case TypeNode<NodeInfo>::Unknown:
//       return "unknown";
//   }
// }

inline bool token_implicit_cast(TokenType from, TokenType to) {
  if (from == to) return true;
  if (from == TokenType::TOKEN_CHAR && (to == TokenType::TOKEN_INT))
    return true;
  else if (from == TokenType::TOKEN_INT &&
           (to == TokenType::TOKEN_CHAR || to == TokenType::TOKEN_STRING))
    return true;

  return false;
}

inline int token_type_to_bit_size(TokenType type) {
  if (type == TokenType::TOKEN_INT) return 32;
  if (type == TokenType::TOKEN_CHAR) return 8;
  return 64;
}

inline TokenType builtin_type_name_to_type(std::string type_name) {
  if (type_name == "int") return TokenType::TOKEN_INT;
  if (type_name == "char") return TokenType::TOKEN_CHAR;

  return TokenType::TOKEN_UNKNOWN;
}

inline int token_to_bit_size(std::string type_name,
                             TokenType fallback = TokenType::TOKEN_UNKNOWN) {
  if (fallback != TokenType::TOKEN_UNKNOWN &&
      fallback != TokenType::TOKEN_DATA_TYPE)
    return token_type_to_bit_size(fallback);
  if (!type_name.find("["))
    return token_type_to_bit_size(
        TokenUtils::builtin_type_name_to_type(type_name));

  size_t idx = type_name.find("[");
  std::string array_type = type_name.substr(0, idx);
  int length = stoi(
      type_name.substr(type_name.find("[") + 1, type_name.length() - idx - 1));

  return token_type_to_bit_size(builtin_type_name_to_type(array_type)) * length;
}

};  // namespace TokenUtils

#endif  // TOKEN_UTILS_H_
