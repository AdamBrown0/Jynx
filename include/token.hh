#ifndef TOKEN_H_
#define TOKEN_H_

#include <memory>
#include <string>
#include <unordered_map>

#define TOKEN_LIST     \
  X(TOKEN_ID)          \
  X(TOKEN_EQUALS)      \
  X(TOKEN_LPAREN)      \
  X(TOKEN_RPAREN)      \
  X(TOKEN_LBRACE)      \
  X(TOKEN_RBRACE)      \
  X(TOKEN_LBRACKET)    \
  X(TOKEN_RBRACKET)    \
  X(TOKEN_COLON)       \
  X(TOKEN_COMMA)       \
  X(TOKEN_LT)          \
  X(TOKEN_GT)          \
  X(TOKEN_LEQ)         \
  X(TOKEN_GEQ)         \
  X(TOKEN_ARROW_RIGHT) \
  X(TOKEN_INT)         \
  X(TOKEN_STRING)      \
  X(TOKEN_STATEMENT)   \
  X(TOKEN_SEMICOLON)   \
  X(TOKEN_PLUS)        \
  X(TOKEN_MINUS)       \
  X(TOKEN_DIVIDE)      \
  X(TOKEN_MULTIPLY)    \
  X(TOKEN_LSHIFT)      \
  X(TOKEN_RSHIFT)      \
  X(TOKEN_COMMENT)     \
  X(TOKEN_EOF)         \
  X(KW_INT)            \
  X(KW_STRING)         \
  X(KW_CLASS)

enum class TokenType {
#define X(name) name,
  TOKEN_LIST
#undef X
};

struct TrieNode {
  std::unordered_map<char, std::unique_ptr<TrieNode>> children;
  bool is_terminal = false;
  TokenType kw_type;  // if (is_terminal)
};

class KeywordTrie {
  std::unique_ptr<TrieNode> root;

 public:
  KeywordTrie() : root(std::make_unique<TrieNode>()) {}

  void insert(const std::string& word, TokenType type) {
    TrieNode* node = root.get();
    for (char c : word) {
      if (!node->children[c]) node->children[c] = std::make_unique<TrieNode>();
      node = node->children[c].get();
    }
    node->is_terminal = true;
    node->kw_type = type;
  }

  TokenType* find(const std::string& word) {
    TrieNode* node = root.get();
    for (char c : word) {
      if (!node->children[c]) return nullptr;
      node = node->children[c].get();
    }
    if (node->is_terminal) return &node->kw_type;
    return nullptr;
  }
};

class Token {
 public:
  Token(TokenType type, std::string value, int line)
      : value(value), type(type), line(line) {}

  TokenType getType() const { return type; }

  void print() {
    std::printf("TokenType: %s Value: %s Line: %d\n", this->to_string(),
                value.c_str(), line);
  }

  constexpr const char* to_string() {
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

 private:
  std::string value;
  TokenType type;
  int line;
};

#endif  // TOKEN_H_
