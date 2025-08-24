#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include "lexer.hh"

std::string file_contents(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file) {
    std::cerr << "Could not open file: " << filepath << std::endl;
    return "";
  }

  std::streamsize file_size = file.tellg();
  if (file_size < 0) {
    std::cerr << "Could not determine size of file: " << filepath << std::endl;
    return "";
  }

  std::string contents(file_size, '\0');
  file.seekg(0, std::ios::beg);
  if (!file.read(&contents[0], file_size)) {
    std::cerr << "Could not read contents of file: " << filepath << std::endl;
    return "";
  }

  return contents;
}

void print_usage(char** argv) {
  std::printf("USAGE: %s <path-to-file>\n", argv[0]);
  exit(1);
}

int main(const int argc, char** argv) {
  if (argc != 2) print_usage(argv);

  std::string contents = file_contents(argv[1]);

  Lexer lexer(contents);

  if (!contents.empty()) {
    std::printf("Contents of %s:\n----\n%s----\n", argv[1], contents.c_str());
  }

}
