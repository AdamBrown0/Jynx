#ifndef TRIE_H_
#define TRIE_H_

#include <memory>
#include <unordered_map>
#include "token.hh"

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

#endif  // TRIE_H_
