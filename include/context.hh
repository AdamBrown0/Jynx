#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "methodtable.hh"
#include "trie.hh"

struct CompilerContext {
  std::unordered_map<std::string, Symbol> symbol_table;
  MethodTable method_table;
  KeywordTrie keywords;
};

#endif  // CONTEXT_H_
