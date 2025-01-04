#ifndef COMPILER_H
#define COMPILER_H

#include "lexer.h"
#include "table.h"
#include "vm.h"
#include <stddef.h>

#define LOCAL_ENV_CAP 255
#define SCOPE_CONTEXT_CAP 255

typedef struct {
  Inst insts[PROGRAM_STACK_CAP];
  uint64_t insts_count;
} Ir;

typedef struct {
  char *str;
  uint64_t len;
} Sv;

typedef struct {
  Ir *ir;

  char *name;

  char *locals[LOCAL_ENV_CAP];
  uint8_t locals_count;

  uint8_t scope;
} Compiler;

typedef struct {
  Token_t op;
  uint8_t pre_power;
  uint8_t in_power;
} Binding_power;

void compiler_init(Compiler *compiler);
void compiler_compile(Compiler *compiler, Token *tokens);
void compiler_dump(Compiler *compiler);

#endif
