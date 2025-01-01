#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "vm.h"

// Resolve to AST

Parser parser;

void parser_load_tokens(Token *tokens, size_t tokens_count) {
  parser.tokens = tokens, parser.tokens_count = tokens_count;
}

inline static void parser_expr_print() {
  parser.insts[parser.inst_count++] = (Inst){
      .type = INST_PRINT,
  };
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

// 1 + 2 * 3 + 4
static void parser_expr_bp(Token *lhs, uint8_t min_bp) {
#define NEXT_TOKEN &parser.tokens[parser.tokens_count++]

  while (1) {
    Token *op = NEXT_TOKEN;

    if (op->type == Token_EOF)
      break;

    uint8_t bp = parser_bp_get(op->type);
    if (bp < min_bp) {
      printf("Op: %s", lexer_token_t_to_str(op->type));
      break;
    }

    printf("%.*s", lhs->len, lhs->start);

    parser_expr_bp(lhs, bp);
  }

#undef NEXT
}

static void parser_expr() {
  Token *token = NEXT_TOKEN;

  if (token->type == Token_Print) {
    parser_expr_print();
  } else if (token->type == Token_Number) {
    parser_expr_bp(token, 0);
  }
}

void parser_parse() { parser_expr(); }

int main(void) {}
