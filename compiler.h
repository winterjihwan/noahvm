#ifndef COMPILER_H
#define COMPILER_H

#include "lexer.h"
#include "table.h"
#include "vm.h"
#include <stddef.h>

typedef struct {
  Token tokens[TOKENS_CAP];
  uint64_t tokens_pos;

  Inst insts[PROGRAM_STACK_CAP];
  uint64_t insts_count;

  HashTable env;
} Compiler;

typedef void (*ParseFn)(void);

typedef struct {
  Token_t op;
  uint8_t pre_power;
  uint8_t in_power;
  ParseFn fn;
} Binding_power;

void compiler_init(void);
void compiler_load_tokens(Token *tokens, size_t tokens_count);
void compiler_compile(void);

#endif
