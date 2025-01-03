#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "lexer.h"
#include "vm.h"

// Resolve to AST

Compiler compiler = {0};

void compiler_init(void) {
  compiler.tokens_pos = 0;
  compiler.insts_count = 0;
}

void compiler_load_tokens(Token *tokens, size_t tokens_count) {
  for (size_t i = 0; i < tokens_count; i++) {
    compiler.tokens[i] = tokens[i];
  }
}

inline static void compiler_expr_print(void) {
  compiler.insts[compiler.insts_count++] = (Inst){
      .type = INST_PRINT,
  };

  compiler.insts_count = 0;
}

#define PRATT_TABLE_CAP 16
static Binding_power compiler_pratt_table[PRATT_TABLE_CAP] = {
    (Binding_power){.op = Token_Plus, .in_power = 1},
    (Binding_power){.op = Token_Minus, .in_power = 2, .pre_power = 5},
    (Binding_power){.op = Token_Mult, .in_power = 3},
    (Binding_power){.op = Token_Div, .in_power = 4},
};

inline static uint8_t compiler_in_power_get(Token_t type) {
  for (size_t i = 0; i < PRATT_TABLE_CAP; i++) {
    if (type == compiler_pratt_table[i].op) {
      uint8_t in_power = compiler_pratt_table[i].in_power;

      return in_power;
    }
  }

  return 0;
}

inline static uint8_t compiler_pre_power_get(Token_t type) {
  for (size_t i = 0; i < PRATT_TABLE_CAP; i++) {
    if (type == compiler_pratt_table[i].op) {
      uint8_t pre_power = compiler_pratt_table[i].pre_power;

      return pre_power;
    }
  }

  return 0;
}

inline static int compiler_token_is_pre_op(Token_t type) {
  if (type == Token_Minus)
    return 1;

  return 0;
}

inline static int compiler_token_ignore_emit(Token_t type) {
  if (compiler_token_is_pre_op(type))
    return 1;

  if (type == Token_LParen)
    return 1;

  return 0;
}

inline static Inst compiler_translate_op(Token_t type) {
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

inline static int compiler_str_view_to_num(char *str, int len) {
  char buf[len + 1];
  snprintf(buf, len + 1, "%.*s", len, str);
  return strtol(buf, NULL, 10);
}

#define NEXT_TOKEN &compiler.tokens[compiler.tokens_pos++]
#define PEEK_TOKEN &compiler.tokens[compiler.tokens_pos]
#define PUSH_INST(inst)                                                        \
  do {                                                                         \
    compiler.insts[compiler.insts_count++] = inst;                             \
  } while (0)
#define MUNCH_TOKEN(token_type)                                                \
  do {                                                                         \
    if (*PEEK_TOKEN.type == token_type) {                                      \
      compiler.tokens_pos++;                                                   \
    } else {                                                                   \
      fprintf(stderr, "Expected token type %s",                                \
              lexer_token_t_to_str(token_type));                               \
      exit(6);                                                                 \
    }                                                                          \
  } while (0)

static void compiler_expr_bp(uint8_t min_bp) {
  Token *lhs = NEXT_TOKEN;

  int lhs_is_pre_op = compiler_token_is_pre_op(lhs->type);
  if (lhs_is_pre_op) {
    uint8_t pre_bp = compiler_pre_power_get(lhs->type);
    compiler_expr_bp(pre_bp);

    PUSH_INST(MAKE_NEGATE);
  } else if (lhs->type == Token_LParen) {
    compiler_expr_bp(0);

    MUNCH_TOKEN(Token_RParen);
  }

  if (!compiler_token_ignore_emit(lhs->type))
    PUSH_INST(MAKE_PUSH(compiler_str_view_to_num(lhs->start, lhs->len)));

  while (1) {
    Token *op = PEEK_TOKEN;

    if (op->type == Token_EOF)
      break;

    uint8_t in_bp = compiler_in_power_get(op->type);

    if (!in_bp)
      break;

    if (in_bp < min_bp) {
      break;
    }

    compiler.tokens_pos++;
    compiler_expr_bp(in_bp);

    PUSH_INST(compiler_translate_op(op->type));
  }
}

static void compiler_expr(void) {
  Token *token = PEEK_TOKEN;

  if (token->type == Token_Print) {
    compiler_expr_print();
  } else {
    compiler_expr_bp(0);
  }
}

void compiler_compile(void) {
  compiler_expr();
  PUSH_INST(MAKE_EOF);
}

#undef PEEK_TOKEN
#undef NEXT_TOKEN
#undef MUNCH_TOKEN
#undef PUSH_INST
