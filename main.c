#include "jsmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER 1024
#define MAX_JSON_TOKENS 128

inline static void scanner_load_code_from_file(const char *file_path,
                                               char *buf) {
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

void scanner_scan(const char *code, jsmntok_t *tokens) {
  jsmn_parser parser;
  jsmn_init(&parser);
  jsmn_parse(&parser, code, strlen(code), tokens, MAX_JSON_TOKENS);
}

int main(int argc, char **argv) {
  if (argc != 2) {

    fprintf(stderr, "USAGE: ./main <file_path>");
    exit(1);
  }

  char *file_path = argv[1];

  char code[MAX_BUFFER] = {0};
  scanner_load_code_from_file(file_path, code);

  jsmntok_t tokens[MAX_JSON_TOKENS];
  scanner_scan(code, tokens);

  jsmn_dump(tokens);
}
