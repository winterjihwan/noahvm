#ifndef COMPILER_H
#define COMPILER_H

#include "lexer.h"
#include "table.h"
#include "vm.h"
#include <stddef.h>

#define LOCAL_ENV_CAP 255
#define SCOPE_CONTEXT_CAP 255
#define FN_CAP 255

typedef struct {
  Inst insts[INSTS_CAP];
  uint64_t insts_count;
} Ir;

typedef struct {
  Sv name;
  uint8_t depth;
} Local;

typedef struct {
  uint64_t label_pos;
  Sv label;

  uint8_t arity;
} Fn;

typedef struct Compiler Compiler;
struct Compiler {
  Compiler *enclosing;

  Ir *ir;

  Sv name;
  uint8_t depth;

  Local locals[LOCAL_ENV_CAP];
  uint8_t locals_count;

  Fn fn[FN_CAP];
  uint8_t fn_count;
};

typedef enum {
  BP_NONE,
  BP_IN_EQ_CMP,
  BP_IN_CMP,
  BP_IN_PLUS,
  BP_IN_MINUS,
  BP_IN_MULT,
  BP_IN_DIV,
  BP_PRE_MINUS,
} Bp_t;

typedef struct {
  uint8_t pre_power;
  uint8_t in_power;
} Bp;

void compiler_init(Compiler *compiler);
void compiler_compile(Compiler *compiler, Token *tokens);

#endif
