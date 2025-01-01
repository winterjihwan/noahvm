#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "vm.h"
#include <stddef.h>

typedef struct {
  Token *tokens;
  uint64_t tokens_count;

  Inst *insts;
  uint64_t inst_count;

} Parser;

typedef void (*ParseFn)();

typedef struct {
  Token_t op;
  uint8_t power;
  ParseFn fn;
} Parser_bp;

#endif
