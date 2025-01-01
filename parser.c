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
    (Parser_bp){.op = Token_Plus, .in_power = 1, .fn = NULL},
    (Parser_bp){.op = Token_Minus, .in_power = 2, .pre_power = 5, .fn = NULL},
    (Parser_bp){.op = Token_Mult, .in_power = 3, .fn = NULL},
    (Parser_bp){.op = Token_Div, .in_power = 4, .fn = NULL},
};

inline static uint8_t parser_in_power_get(Token_t type) {
  for (size_t i = 0; i < PRATT_TABLE_CAP; i++) {
    if (type == parser_pratt_table[i].op) {
      uint8_t in_power = parser_pratt_table[i].in_power;
      if (in_power == 0)
        goto panic;

      return in_power;
    }
  }

panic:
  fprintf(stderr, "None binding power for op");
  exit(4);
}

inline static uint8_t parser_pre_power_get(Token_t type) {
  for (size_t i = 0; i < PRATT_TABLE_CAP; i++) {
    if (type == parser_pratt_table[i].op) {
      uint8_t pre_power = parser_pratt_table[i].pre_power;
      if (pre_power == 0)
        goto panic;

      return pre_power;
    }
  }

panic:
  fprintf(stderr, "None binding power for op");
  exit(4);
}

inline static int parser_token_is_op(Token_t type) {
  if (type == Token_Plus || type == Token_Minus || type == Token_Mult ||
      type == Token_Div)
    return 1;

  return 0;
}

inline static Inst parser_translate_op(Token_t type) {
  switch (type) {
  case Token_Plus:
    return MAKE_PLUS;
  case Token_Minus:
    return MAKE_MINUS;
  case Token_Mult:
    return MAKE_MULT;
  case Token_Div:
    return MAKE_DIV;
  default:
    fprintf(stderr, "Expected op");
    exit(5);
  };
}

inline static int parser_str_view_to_num(char *str, int len) {
  char buf[len + 1];
  snprintf(buf, len + 1, "%.*s", len, str);
  return strtol(buf, NULL, 10);
}

#define NEXT_TOKEN &parser.tokens[parser.tokens_pos++]
#define PEEK_TOKEN &parser.tokens[parser.tokens_pos]
#define PUSH_INST(inst)                                                        \
  do {                                                                         \
    parser.insts[parser.insts_count++] = inst;                                 \
  } while (0)

static void parser_expr_bp(uint8_t min_bp) {
  Token *lhs = NEXT_TOKEN;
  int lhs_is_op = parser_token_is_op(lhs->type);
  if (lhs_is_op) {
    uint8_t pre_bp = parser_pre_power_get(lhs->type);
    parser_expr_bp(pre_bp);

    PUSH_INST(MAKE_NEGATE);
  }

  if (!lhs_is_op)
    PUSH_INST(MAKE_PUSH(parser_str_view_to_num(lhs->start, lhs->len)));

  while (1) {
    Token *op = PEEK_TOKEN;

    if (op->type == Token_EOF)
      break;

    uint8_t in_bp = parser_in_power_get(op->type);
    if (in_bp < min_bp) {
      break;
    }

    parser.tokens_pos++;
    parser_expr_bp(in_bp);

    PUSH_INST(parser_translate_op(op->type));
  }
}

static void parser_expr(void) {

  Token *token = PEEK_TOKEN;

  if (token->type == Token_Print) {
    parser_expr_print();
  } else {
    parser_expr_bp(0);
  }
}

#undef PEEK_TOKEN
#undef NEXT_TOKEN

void parser_parse(void) {
  parser_expr();
  PUSH_INST(MAKE_EOF);
}

#undef PUSH_INST
