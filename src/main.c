#include <errno.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define extern_
#include "data.h"
#undef extern_

#include <macros.h>

char* file_contents(FILE* file) {
  // Open the file
  // Determine its size by seeking to the end
  fseek(file, 0, SEEK_END);
  const long file_size = ftell(file);
  if (file_size < 0) {
    printf("Could not determine size of file %p: %s\n", file, strerror(errno));
    return NULL;
  }

  // Read the contents into a string
  char* contents = malloc(file_size + 1);
  rewind(file);
  if (fread(contents, 1, file_size, file) != file_size) {
    free(contents);
    printf("Could not read contents of file %p: %s\n", file, strerror(errno));
    return NULL;
  }
  contents[file_size] = 0;

  // Close the file
  fclose(file);

  return contents;
}

void print_usage(char** argv) {
  printf("USAGE: %s <path-to-file>\n", argv[0]);
  exit(1);
}

int main(const int argc, char** argv) {
  if (argc != 2)
    print_usage(argv);

  lexer_t* lexer = init_lexer(argv[1]);

  char* contents = file_contents(Infile);

  if (contents) {
    printf("Contents of %s:\n----\n%s\n----\n", argv[1], contents);
    free(contents);
  }
  return 0;
}
