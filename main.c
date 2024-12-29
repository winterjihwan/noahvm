#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

#define MAX_BUFFER 1024
#define MAX_JSON_TOKENS 128

inline static void load_code_from_file(const char *file_path, char *buf) {
  FILE *file = fopen(file_path, "rb");

  if (fseek(file, 0, SEEK_END) < 0) {
    printf("ERROR: fseek");
    goto close;
  }

  long m = ftell(file);
  if (m < 0) {
    printf("ERROR: ftell");
    goto close;
  }

  if (fseek(file, 0, SEEK_SET) < 0) {
    printf("ERROR: fseek");
    goto close;
  }

  fread(buf, 1, (size_t)m, file);
  if (ferror(file)) {
    printf("ERROR: fread");
    goto close;
  }

  fclose(file);

close:
  if (file)
    fclose(file);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "USAGE: ./main <file.c>");
    exit(1);
  }

  char *code_path = argv[1];

  char code[MAX_BUFFER] = {0};
  load_code_from_file(code_path, code);

  Token tokens[512] = {0};
  lexer_lex(code, tokens);

  printf("Code: %s", code);
}
