#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "vm.h"
#include <stddef.h>

typedef struct {
  Token tokens[TOKENS_CAP];
  uint64_t tokens_pos;

  Inst insts[PROGRAM_STACK_CAP];
  uint64_t insts_count;

} Parser;

typedef void (*ParseFn)(void);

typedef struct {
  Token_t op;
  uint8_t power;
  ParseFn fn;
} Parser_bp;

void parser_init(void);
void parser_load_tokens(Token *tokens, size_t tokens_count);
void parser_parse(void);

#endif
