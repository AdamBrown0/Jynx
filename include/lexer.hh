#ifndef LEXER_HH
#define LEXER_HH

#include <cstring>
#include <string>

class Lexer {
public:
  Lexer(std::string src) {
    this->src = src;
    src_size = std::strlen(src.c_str());
    index = 0;
    current = src[index];
  }

 private:
  std::string src;
  size_t src_size;
  char current;
  unsigned int index;
};

#endif  // LEXER_HH
