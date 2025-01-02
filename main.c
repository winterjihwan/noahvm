#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "lexer.h"
#include "vm.h"

#define CODE_CAP 1024

inline static void null_terminate(char *str) { str[strlen(str) - 1] = '\0'; }

static void load_code_from_file(const char *file_path, char *buf) {
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

  null_terminate(buf);

  fclose(file);

close:
  if (file)
    fclose(file);
}

extern Lexer lexer;
extern Compiler compiler;
extern Vm vm;

int main(void) {
#define INST_COUNT 5

  Word label_a = (Word){.as_str = "a"};

  Inst program[INST_COUNT] = {
      MAKE_PUSH(3), MAKE_PUSH(3), MAKE_PLUS, MAKE_ASSIGN((Word){.as_str = "a"}),
      MAKE_EOF,
  };

  vm_init();
  vm_program_load_from_memory(program, INST_COUNT);
  vm_program_dump();
  vm_execute();
  vm_stack_dump();

#undef INST_COUNT
}

/*int main_2(int argc, char **argv) {*/
/*  if (argc != 2) {*/
/*    fprintf(stderr, "USAGE: ./main <file.c>");*/
/*    exit(1);*/
/*  }*/
/**/
/*  char *code_path = argv[1];*/
/**/
/*  char code[CODE_CAP] = {0};*/
/*  load_code_from_file(code_path, code);*/
/**/
/*  printf("Code: %s\n", code);*/
/**/
/*  lexer_init_with_code(code);*/
/*  lexer_lex();*/
/*  lexer_tokens_dump(lexer.tokens);*/
/**/
/*  compiler_init();*/
/*  compiler_load_tokens(lexer.tokens, lexer.tokens_count);*/
/*  compiler_compile();*/
/**/
/*  vm_init();*/
/*  vm_program_load_from_memory(compiler.insts, compiler.insts_count);*/
/*  vm_program_dump();*/
/*  vm_stack_dump();*/
/*  vm_execute();*/
/*  vm_stack_dump();*/
/*}*/
