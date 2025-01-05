#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "lexer.h"
#include "table.h"
#include "vm.h"

static uint64_t tokens_pos = 0;
static Ir ir = {0};

void compiler_init(Compiler *compiler) {
  compiler->ir = &ir;
  compiler->name = (Sv){
      .len = 4,
      .str = "main",
  };
  compiler->depth = 0;
  compiler->enclosing = NULL;
}

void compiler_dump(Compiler *compiler) {
  printf("%.*s:\n", compiler->name.len, compiler->name.str);
  printf("\tdepth: %d\n", compiler->depth);
}

static Bp PRED_TABLE[Token_EOF + 1] = {
    [Token_Plus] =
        {
            .in_power = BP_IN_PLUS,
        },
    [Token_Minus] =
        {
            .in_power = BP_IN_MINUS,
            .pre_power = BP_PRE_MINUS,
        },
    [Token_Mult] =
        {
            .in_power = BP_IN_MULT,
        },
    [Token_Div] =
        {
            .in_power = BP_IN_DIV,
        },
};

inline static int compiler_token_is_pre_op(Token_t type) {
  if (type == Token_Minus)
    return 1;

  return 0;
}

static Inst compiler_translate_op(Token_t lhs_type, Token_t type) {
  int as_f = lhs_type == Token_Float ? 1 : 0;

  switch (type) {
  case Token_Plus:
    return as_f ? MAKE_PLUSF : MAKE_PLUS;
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

inline static uint64_t compiler_sv_to_u64(char *str, int len) {
  char buf[len + 1];
  snprintf(buf, len + 1, "%.*s", len, str);
  return strtol(buf, NULL, 10);
}

inline static uint64_t compiler_sv_to_f64(char *str, int len) {
  char buf[len + 1];
  snprintf(buf, len + 1, "%.*s", len, str);
  return strtof(buf, NULL);
}

#define NEXT_TOKEN &tokens[tokens_pos++]
#define PEEK_TOKEN &tokens[tokens_pos]
#define PUSH_INST(inst)                                                        \
  do {                                                                         \
    compiler->ir->insts[compiler->ir->insts_count++] = inst;                   \
  } while (0)
#define ALTER_INST(offset, inst)                                               \
  do {                                                                         \
    compiler->ir->insts[offset] = inst;                                        \
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
      if (memcmp(local->name.str, name->str, name->len) == 0) {
        return i;
      }
    }
  }

  return -1;
}

static void compiler_emit_ir(Compiler *compiler, Token *lhs) {
  if (lhs->type == Token_Number) {
    Word operand = (Word){.as_u64 = compiler_sv_to_u64(lhs->start, lhs->len)};

    PUSH_INST(MAKE_PUSH(operand));
  } else if (lhs->type == Token_Float) {
    Word operand = (Word){.as_f64 = compiler_sv_to_f64(lhs->start, lhs->len)};

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

    if (!in_bp || in_bp < min_bp) {
      break;
    }

    tokens_pos++;
    compiler_expr_bp(compiler, tokens, in_bp);

    PUSH_INST(compiler_translate_op(lhs->type, op->type));
  }
}

static void compiler_expr(Compiler *compiler, Token *tokens) {
  compiler_expr_bp(compiler, tokens, 0);
}

static void compiler_stmt_print(Compiler *compiler, Token *tokens) {
  MUNCH_TOKEN(Token_Print);

  compiler_expr(compiler, tokens);

  PUSH_INST(MAKE_PRINT);
  PUSH_INST(MAKE_POP);
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

static void compiler_stmt_assign(Compiler *compiler, Token *tokens, Sv name) {
  MUNCH_TOKEN(Token_Equal);

  compiler_expr(compiler, tokens);

  if (compiler->depth == 0) {
    PUSH_INST(MAKE_DEF_GLOBAL(name));
  } else {
    LOCAL_ADD(name, compiler->depth);

    int offset = compiler_var_resolve(compiler, &name);
    PUSH_INST(MAKE_DEF_LOCAL(offset));
  }

  PUSH_INST(MAKE_POP);
  MUNCH_TOKEN(Token_Semicolon);
}

static void compiler_stmt_define(Compiler *compiler, Token *tokens) {
  // TODO: Type checking
  Token *type = NEXT_TOKEN;

  EXPECT_TOKEN(Token_Identifier);
  Token *identifier = NEXT_TOKEN;

  Sv name = (Sv){
      .str = identifier->start,
      .len = identifier->len,
  };

  compiler_stmt_assign(compiler, tokens, name);
}

static void compiler_stmt(Compiler *compiler, Token *tokens);

#define ENTER_SCOPE                                                            \
  do {                                                                         \
    MUNCH_TOKEN(Token_LBrace);                                                 \
    compiler->depth++;                                                         \
  } while (0)
#define EXIT_SCOPE                                                             \
  do {                                                                         \
    compiler->locals_count = locals_count_prev;                                \
    compiler->depth--;                                                         \
    MUNCH_TOKEN(Token_RBrace);                                                 \
  } while (0)

static void compiler_stmt_block(Compiler *compiler, Token *tokens) {
  ENTER_SCOPE;

  uint8_t locals_count_prev = compiler->locals_count;

  while (1) {
    Token *peek = PEEK_TOKEN;
    if (peek->type == Token_EOF || peek->type == Token_RBrace)
      break;

    compiler_stmt(compiler, tokens);
  }

  for (size_t i = 0; i < compiler->locals_count - locals_count_prev; i++) {
    PUSH_INST(MAKE_POP);
  }

  EXIT_SCOPE;
}

static Compiler *compiler_new(Compiler *compiler, Sv name) {
  Compiler *new_fn = (Compiler *)malloc(sizeof(Compiler));
  new_fn->depth = compiler->depth + 1;
  new_fn->name = name;
  new_fn->locals_count = 0;
  new_fn->enclosing = compiler;

  return new_fn;
}

#define FN_DEF(_label, _label_pos, _arity, _params)                            \
  do {                                                                         \
    compiler->fn[compiler->fn_count++] = (Fn){                                 \
        .label = _label,                                                       \
        .label_pos = _label_pos,                                               \
        .arity = _arity,                                                       \
        .params = _params,                                                     \
    };                                                                         \
  } while (0);

static void compiler_stmt_fn(Compiler *compiler, Token *tokens) {
  MUNCH_TOKEN(Token_Fn);

  EXPECT_TOKEN(Token_Identifier);
  Token *identifier = NEXT_TOKEN;
  Sv label = (Sv){
      .str = identifier->start,
      .len = identifier->len,
  };

  PUSH_INST(MAKE_JMP_ABS(-1));

  uint64_t label_start_pos = compiler->ir->insts_count;

  MUNCH_TOKEN(Token_LParen);

  Sv params[UINT8_MAX];
  uint8_t params_count = 0;
  while (1) {
    // TODO: Type checking
    Token *next = NEXT_TOKEN;

    if (next->type == Token_RParen)
      break;

    EXPECT_TOKEN(Token_Identifier);
    Token *identifier = NEXT_TOKEN;

    params[params_count++] = (Sv){
        .len = identifier->len,
        .str = identifier->start,
    };

    if (*PEEK_TOKEN.type == Token_RParen)
      break;

    MUNCH_TOKEN(Token_Comma);
  }

  FN_DEF(label, label_start_pos, params_count - 1, *params);

  compiler_stmt_block(compiler, tokens);

  uint64_t label_end_pos = compiler->ir->insts_count;
  ALTER_INST(label_start_pos - 1, MAKE_JMP_ABS(label_end_pos));
}

static void compiler_stmt_call(Compiler *compiler, Token *tokens) {}

static void compiler_stmt_var(Compiler *compiler, Token *tokens) {
  Token *identifier = NEXT_TOKEN;

  Sv name = (Sv){
      .str = identifier->start,
      .len = identifier->len,
  };

  if (*PEEK_TOKEN.type == Token_Semicolon) {
    MUNCH_TOKEN(Token_Semicolon);
    return;
  }

  Token *peek = PEEK_TOKEN;
  if (peek->type == Token_Equal) {
    compiler_stmt_assign(compiler, tokens, name);
  } else if (peek->type == Token_LParen) {
    compiler_stmt_call(compiler, tokens);
  }
}

static void compiler_stmt(Compiler *compiler, Token *tokens) {
  Token *peek = PEEK_TOKEN;

  if (peek->type == Token_Int || peek->type == Token_Str) {
    compiler_stmt_define(compiler, tokens);

  } else if (peek->type == Token_Identifier) {
    compiler_stmt_var(compiler, tokens);

  } else if (peek->type == Token_Print) {
    compiler_stmt_print(compiler, tokens);

  } else if (peek->type == Token_LBrace) {
    compiler_stmt_block(compiler, tokens);

  } else if (peek->type == Token_Fn) {
    compiler_stmt_fn(compiler, tokens);

  } else {
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
#undef ALTER_INST
#undef POP_INST

#undef LOCAL_ADD
#undef EXPECT_TOKEN

#undef ENTER_SCOPE
#undef EXIT_SCOPE

#undef FN_DEF
