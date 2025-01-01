#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "vm.h"

// Resolve to AST

Parser parser = {0};

void parser_init(void) {
  parser.tokens_pos = 0;
  parser.insts_count = 0;
}

void parser_load_tokens(Token *tokens, size_t tokens_count) {
  for (size_t i = 0; i < tokens_count; i++) {
    parser.tokens[i] = tokens[i];
  }
}

inline static void parser_expr_print(void) {
  parser.insts[parser.insts_count++] = (Inst){
      .type = INST_PRINT,
  };

  parser.insts_count = 0;
}

#define PRATT_TABLE_CAP 16
static Parser_bp parser_pratt_table[PRATT_TABLE_CAP] = {
    (Parser_bp){.op = Token_Number, .power = 0, .fn = NULL},
    (Parser_bp){.op = Token_Plus, .power = 1, .fn = NULL},
    (Parser_bp){.op = Token_Minus, .power = 2, .fn = NULL},
    (Parser_bp){.op = Token_Mult, .power = 3, .fn = NULL},
    (Parser_bp){.op = Token_Div, .power = 4, .fn = NULL},
};

inline static uint8_t parser_bp_get(Token_t type) {
  for (size_t i = 0; i < PRATT_TABLE_CAP; i++) {
    if (type == parser_pratt_table[i].op) {
      return parser_pratt_table[i].power;
    }
  }

  fprintf(stderr, "None binding power for op");
  exit(4);
}

#define NEXT_TOKEN &parser.tokens[parser.tokens_pos++]
#define PEEK_TOKEN &parser.tokens[parser.tokens_pos]
#define PUSH_INST(inst)                                                        \
  do {                                                                         \
    parser.insts[parser.insts_count++] = inst;                                 \
  } while (0)

static void parser_expr_bp(uint8_t min_bp) {
  Token *lhs = NEXT_TOKEN;

  // Push number
  const Inst inst = (Inst){.type = INST_PUSH, .operand = lhs->len};
  PUSH_INST(inst);

  while (1) {
    Token *op = PEEK_TOKEN;

    if (op->type == Token_EOF)
      break;

    uint8_t bp = parser_bp_get(op->type);
    if (bp < min_bp) {
      break;
    }

    parser.tokens_pos++;
    parser_expr_bp(bp);

    // Push op
    const Inst inst = (Inst){.type = INST_PUSH, .operand = lhs->len};
    PUSH_INST(inst);
  }
}

static void parser_expr(void) {

  Token *token = PEEK_TOKEN;

  if (token->type == Token_Print) {
    parser_expr_print();
  } else if (token->type == Token_Number) {
    parser_expr_bp(0);
  }
}

#undef PEEK_TOKEN
#undef NEXT_TOKEN

void parser_parse(void) { parser_expr(); }
