#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "lexer.h"
#include "vm.h"

static uint64_t tokens_pos = 0;
static Ir ir = {0};

void compiler_init(Compiler *compiler) {
  compiler->ir = &ir;
  compiler->name = "main";
  compiler->depth = 0;
  compiler->enclosing = NULL;
}

void compiler_dump(Compiler *compiler) {
  printf("%s:\n", compiler->name);
  printf("\tdepth: %d\n", compiler->depth);
}

static Bp PRED_TABLE[Token_EOF] = {
    [Token_Plus] = {.in_power = BP_IN_PLUS},
    [Token_Minus] = {.in_power = BP_IN_MINUS, .pre_power = BP_PRE_MINUS},
    [Token_Mult] = {.in_power = BP_IN_MULT},
    [Token_Div] = {.in_power = BP_IN_DIV},
};

inline static int compiler_token_is_pre_op(Token_t type) {
  if (type == Token_Minus)
    return 1;

  return 0;
}

static Inst compiler_translate_op(Token_t type) {
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

inline static int compiler_sv_to_num(char *str, int len) {
  char buf[len + 1];
  snprintf(buf, len + 1, "%.*s", len, str);
  return strtol(buf, NULL, 10);
}

#define NEXT_TOKEN &tokens[tokens_pos++]
#define PEEK_TOKEN &tokens[tokens_pos]
#define PUSH_INST(inst)                                                        \
  do {                                                                         \
    compiler->ir->insts[compiler->ir->insts_count++] = inst;                   \
  } while (0)
#define MUNCH_TOKEN(token_type)                                                \
  do {                                                                         \
    if (*PEEK_TOKEN.type == token_type) {                                      \
      tokens_pos++;                                                            \
    } else {                                                                   \
      fprintf(stderr, "Expected token type %s",                                \
              lexer_token_t_to_str(token_type));                               \
      exit(6);                                                                 \
    }                                                                          \
  } while (0)

static int compiler_var_resolve(Compiler *compiler, Sv *name) {
  for (int i = compiler->locals_count - 1; i >= 0; i--) {
    Local *local = &compiler->locals[i];

    if (local->name.len == name->len && local->depth <= compiler->depth) {
      if (memcpy(local->name.str, name->str, name->len)) {
        return i;
      }
    }
  }

  return -1;
}

static void compiler_emit_ir(Compiler *compiler, Token *lhs) {
  if (lhs->type == Token_Number) {
    Word operand = (Word){.as_u64 = compiler_sv_to_num(lhs->start, lhs->len)};

    PUSH_INST(MAKE_PUSH(operand));
  } else if (lhs->type == Token_Identifier) {
    Sv name = (Sv){
        .str = lhs->start,
        .len = lhs->len,
    };
    int offset = compiler_var_resolve(compiler, &name);

    if (offset == -1) {
      PUSH_INST(MAKE_VAR_GLOBAL(name));
    } else {
      PUSH_INST(MAKE_VAR_LOCAL(offset));
    }
  }
}

static void compiler_expr_bp(Compiler *compiler, Token *tokens,
                             uint8_t min_bp) {
  Token *lhs = NEXT_TOKEN;

  int lhs_is_pre_op = compiler_token_is_pre_op(lhs->type);
  if (lhs_is_pre_op) {
    uint8_t pre_bp = PRED_TABLE[lhs->type].pre_power;
    compiler_expr_bp(compiler, tokens, pre_bp);

    PUSH_INST(MAKE_NEGATE);
  } else if (lhs->type == Token_LParen) {
    compiler_expr_bp(compiler, tokens, 0);

    MUNCH_TOKEN(Token_RParen);
  }

  compiler_emit_ir(compiler, lhs);

  while (1) {
    Token *op = PEEK_TOKEN;
    if (op->type == Token_EOF)
      break;

    uint8_t in_bp = PRED_TABLE[op->type].in_power;

    if (!in_bp)
      break;

    if (in_bp < min_bp) {
      break;
    }

    tokens_pos++;
    compiler_expr_bp(compiler, tokens, in_bp);

    PUSH_INST(compiler_translate_op(op->type));
  }
}

static void compiler_expr(Compiler *compiler, Token *tokens) {
  compiler_expr_bp(compiler, tokens, 0);
}

static void compiler_stmt_print(Compiler *compiler, Token *tokens) {
  MUNCH_TOKEN(Token_Print);

  compiler_expr(compiler, tokens);

  PUSH_INST(MAKE_PRINT);

  MUNCH_TOKEN(Token_Semicolon);
}

static void compiler_expr_stmt(Compiler *compiler, Token *tokens) {
  compiler_expr(compiler, tokens);

  PUSH_INST(MAKE_POP);
  MUNCH_TOKEN(Token_Semicolon);
}

#define LOCAL_ADD(_name, _depth)                                               \
  do {                                                                         \
    compiler->locals[compiler->locals_count++] =                               \
        (Local){.name = _name, .depth = _depth};                               \
  } while (0)
#define EXPECT_TOKEN(expected_type)                                            \
  do {                                                                         \
    if (*PEEK_TOKEN.type != expected_type) {                                   \
      fprintf(stderr, "Expected type %s",                                      \
              lexer_token_t_to_str(expected_type));                            \
      exit(7);                                                                 \
    }                                                                          \
  } while (0)

static void compiler_stmt_assign(Compiler *compiler, Token *tokens) {
  Token *identifier = NEXT_TOKEN;

  if (*PEEK_TOKEN.type == Token_Semicolon) {
    MUNCH_TOKEN(Token_Semicolon);
    return;
  }

  MUNCH_TOKEN(Token_Equal);

  compiler_expr(compiler, tokens);

  Sv name = (Sv){
      .str = identifier->start,
      .len = identifier->len,
  };

  if (compiler->depth == 0) {
    // Global
    PUSH_INST(MAKE_DEFINE(name));
  } else {
    LOCAL_ADD(name, compiler->depth);
  }

  MUNCH_TOKEN(Token_Semicolon);
}

static void compiler_stmt_define(Compiler *compiler, Token *tokens) {
  // TODO: Type checking
  Token *type = NEXT_TOKEN;

  EXPECT_TOKEN(Token_Identifier);

  compiler_stmt_assign(compiler, tokens);
}

static void compiler_stmt(Compiler *compiler, Token *tokens);

static void compiler_stmt_block(Compiler *compiler, Token *tokens) {
  MUNCH_TOKEN(Token_LBrace);
  compiler->depth++;

  while (1) {
    Token *peek = PEEK_TOKEN;
    if (peek->type == Token_EOF || peek->type == Token_RBrace)
      break;

    compiler_stmt(compiler, tokens);
  }

  compiler->depth--;
  MUNCH_TOKEN(Token_RBrace);
}

static void compiler_stmt(Compiler *compiler, Token *tokens) {
  Token *peek = PEEK_TOKEN;

  if (peek->type == Token_Int || peek->type == Token_Str) {
    compiler_stmt_define(compiler, tokens);

  } else if (peek->type == Token_Identifier) {
    compiler_stmt_assign(compiler, tokens);

  } else if (peek->type == Token_Print) {
    compiler_stmt_print(compiler, tokens);

  } else if (peek->type == Token_LBrace) {
    compiler_stmt_block(compiler, tokens);

  } else if (peek->type == Token_Number) {
    compiler_expr_stmt(compiler, tokens);
  };
}

void compiler_compile(Compiler *compiler, Token *tokens) {
  while (1) {
    Token *peek = PEEK_TOKEN;
    if (peek->type == Token_EOF)
      break;

    compiler_stmt(compiler, tokens);
  }

  PUSH_INST(MAKE_EOF);
}

#undef PEEK_TOKEN
#undef NEXT_TOKEN
#undef MUNCH_TOKEN
#undef PUSH_INST
#undef POP_INST

#undef LOCAL_ADD
#undef EXPECT_TOKEN
