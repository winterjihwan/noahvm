#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "compiler.h"
#include "lexer.h"
#include "vm.h"

#define CODE_CAP 1024

static void load_code_from_file(const char *file_path, char *buf) {
#define NULL_TERMINATE(str)                                                    \
  do {                                                                         \
    str[strlen(str) - 1] = '\0';                                               \
  } while (0)

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

  NULL_TERMINATE(buf);

  fclose(file);
  return;

close:
  if (file)
    fclose(file);
#undef NULL_TERMINATE
}

extern Lexer lexer;
extern Compiler compiler;
extern Analyzer analyzer;
extern Vm vm;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "USAGE: ./main <file.c>");
    exit(1);
  }

  char *code_path = argv[1];

  char code[CODE_CAP] = {0};
  load_code_from_file(code_path, code);

  printf("Code: \n%s\n\n", code);

  lexer_init_with_code(code);
  lexer_lex();
  lexer_tokens_dump(lexer.tokens);

  Compiler compiler;
  compiler_init(&compiler);
  compiler_compile(&compiler, lexer.tokens);

  analyzer_analyze_basic_blocks(compiler.ir->insts);

  vm_init();
  vm_program_load_from_memory(compiler.ir->insts, compiler.ir->insts_count);
  vm_program_dump();
  vm_execute();
  vm_stack_dump();
  vm_destruct();
}
