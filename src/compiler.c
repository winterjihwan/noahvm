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

__attribute__((unused)) static void compiler_locals_dump(Compiler *compiler) {
  printf("Locals: \n");
  for (size_t i = 0; i < compiler->locals_count; i++) {
    Local *local = &compiler->locals[i];

    printf("%.*s:\n", local->name.len, local->name.str);
    printf("\tdepth: %d\n", local->depth);
  }
}

__attribute__((unused)) static void compiler_dump(const Compiler *compiler) {
  printf("%.*s:\n", compiler->name.len, compiler->name.str);
  printf("\tdepth: %d\n", compiler->depth);
}

static Bp PRED_TABLE[Token_EOF + 1] = {
    [Token_EqualEqual] =
        {
            .in_power = BP_IN_EQ_CMP,
        },
    [Token_ExcMarkEqual] =
        {
            .in_power = BP_IN_EQ_CMP,
        },
    [Token_GT] =
        {
            .in_power = BP_IN_CMP,
        },
    [Token_LT] =
        {
            .in_power = BP_IN_CMP,
        },
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

inline static int compiler_token_is_pre_op(const Token_t type) {
  if (type == Token_Minus)
    return 1;

  return 0;
}

static Inst compiler_translate_op(const Token_t lhs_type, const Token_t type) {
  int as_f = lhs_type == Token_Float ? 1 : 0;

  switch (type) {
  case Token_EqualEqual:
    return MAKE_EQ;
  case Token_ExcMarkEqual:
    return MAKE_NE;
  case Token_GT:
    return MAKE_GT;
  case Token_LT:
    return MAKE_LT;
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

inline static uint64_t compiler_sv_to_u64(const char *str, const int len) {
  char buf[len + 1];
  snprintf(buf, len + 1, "%.*s", len, str);
  return strtol(buf, NULL, 10);
}

inline static uint64_t compiler_sv_to_f64(const char *str, const int len) {
  char buf[len + 1];
  snprintf(buf, len + 1, "%.*s", len, str);
  return strtof(buf, NULL);
}

#define NEXT_TOKEN &tokens[tokens_pos++]
#define PEEK_TOKEN &tokens[tokens_pos]
#define PEEK_TOKEN_TYPE tokens[tokens_pos].type
#define PEEK_PEEK_TOKEN &tokens[tokens_pos + 1]
#define LOC_INST compiler->ir->insts_count
#define PUSH_INST(inst)                                                        \
  do {                                                                         \
    compiler->ir->insts[LOC_INST++] = inst;                                    \
  } while (0)
#define ALTER_INST(offset, inst)                                               \
  do {                                                                         \
    compiler->ir->insts[offset] = inst;                                        \
  } while (0)
#define MUNCH_TOKEN(token_type)                                                \
  do {                                                                         \
    if (PEEK_TOKEN_TYPE == token_type) {                                       \
      tokens_pos++;                                                            \
    } else {                                                                   \
      fprintf(stderr, "Expected token type %s",                                \
              lexer_token_t_to_str(token_type));                               \
      exit(6);                                                                 \
    }                                                                          \
  } while (0)

static int compiler_var_resolve(Compiler *compiler, const Sv *name) {
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

static void compiler_emit_ir(Compiler *compiler, const Token *lhs) {
  if (lhs->type == Token_Number) {
    Word operand = (Word){.as_u64 = compiler_sv_to_u64(lhs->start, lhs->len)};
    PUSH_INST(MAKE_PUSH(operand));

  } else if (lhs->type == Token_Literal) {
    Sv literal = {.len = lhs->len, .str = lhs->start};
    Word operand = (Word){.as_sv = literal};
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
      PUSH_INST(MAKE_VARG(name));
    } else {
      PUSH_INST(MAKE_VARL(offset));
    }
  }
}

static void compiler_expr_call(Compiler *compiler, Token *tokens,
                               const Sv label);
static void compiler_expr_bp(Compiler *compiler, Token *tokens,
                             const uint8_t min_bp) {
  Token *lhs = NEXT_TOKEN;
  if (lhs->type == Token_Quote) {
    lhs = NEXT_TOKEN;
    MUNCH_TOKEN(Token_Quote);
  }

  if (PEEK_TOKEN_TYPE == Token_LParen) {
    Sv label = {.len = lhs->len, .str = lhs->start};

    compiler_expr_call(compiler, tokens, label);

    lhs = PEEK_TOKEN;
  }

  int lhs_is_pre_op = compiler_token_is_pre_op(lhs->type);
  if (lhs_is_pre_op) {
    uint8_t pre_bp = PRED_TABLE[lhs->type].pre_power;
    compiler_expr_bp(compiler, tokens, pre_bp);

    PUSH_INST(MAKE_NEG);
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
    if (PEEK_TOKEN_TYPE != expected_type) {                                    \
      fprintf(stderr, "Expected type %s",                                      \
              lexer_token_t_to_str(expected_type));                            \
      exit(7);                                                                 \
    }                                                                          \
  } while (0)

static void compiler_stmt_assign(Compiler *compiler, Token *tokens) {
  EXPECT_TOKEN(Token_Identifier);
  Token *identifier = NEXT_TOKEN;
  Sv name = (Sv){
      .str = identifier->start,
      .len = identifier->len,
  };

  MUNCH_TOKEN(Token_Equal);
  compiler_expr(compiler, tokens);

  int offset = compiler_var_resolve(compiler, &name);
  if (offset == -1) {
    // defg #name
    PUSH_INST(MAKE_DEFG(name));

  } else {
    // defl #name
    PUSH_INST(MAKE_DEFL(compiler->locals_count - 1));
    LOCAL_ADD(name, compiler->depth);
  }

  PUSH_INST(MAKE_POP);
  MUNCH_TOKEN(Token_Semicolon);
}

static void compiler_stmt_define(Compiler *compiler, Token *tokens) {
  // TODO: Type checking
  Token *type = NEXT_TOKEN;
  (void)type;

  compiler_stmt_assign(compiler, tokens);
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

static void compiler_stmt_if(Compiler *compiler, Token *tokens) {
  MUNCH_TOKEN(Token_If);

  MUNCH_TOKEN(Token_LParen);
  compiler_expr(compiler, tokens);
  MUNCH_TOKEN(Token_RParen);

  // jmpnt #offset
  PUSH_INST(MAKE_JMPNT(-1));
  uint64_t then_start_pos = LOC_INST;

  compiler_stmt_block(compiler, tokens);
  uint64_t then_end_pos = LOC_INST;

  // jmpa #offset
  PUSH_INST(MAKE_JMPA(then_end_pos + 1));
  ALTER_INST(then_start_pos - 1, MAKE_JMPNT(then_end_pos + 1));

  if (PEEK_TOKEN_TYPE == Token_Else) {
    MUNCH_TOKEN(Token_Else);

    compiler_stmt_block(compiler, tokens);

    uint64_t else_end_pos = LOC_INST;
    ALTER_INST(then_end_pos, MAKE_JMPA(else_end_pos));
  }
}

static void compiler_stmt_while(Compiler *compiler, Token *tokens) {
  MUNCH_TOKEN(Token_While);

  MUNCH_TOKEN(Token_LParen);
  uint64_t while_pred_start_pos = LOC_INST;
  compiler_expr(compiler, tokens);

  printf("Then what is it? %.*s\n", *PEEK_TOKEN.len, *PEEK_TOKEN.start);

  MUNCH_TOKEN(Token_RParen);

  // jmpnt #offset
  PUSH_INST(MAKE_JMPNT(-1));
  uint64_t while_start_pos = LOC_INST;

  compiler_stmt_block(compiler, tokens);

  // jmpa #offset
  PUSH_INST(MAKE_JMPA(while_pred_start_pos));

  uint64_t while_end_pos = LOC_INST;
  ALTER_INST(while_start_pos - 1, MAKE_JMPNT(while_end_pos));
}

static Compiler *compiler_call_new(Compiler *compiler, Sv name) {
  Compiler *new_fn = (Compiler *)malloc(sizeof(Compiler));
  new_fn->depth = compiler->depth + 1;
  new_fn->name = name;
  new_fn->locals_count = 0;
  new_fn->enclosing = compiler;
  new_fn->ir = compiler->ir;

  return new_fn;
}

static void compiler_call_destruct(Compiler *compiler) {
  if (compiler->enclosing == NULL) {
    fprintf(stderr, "Enclosing compiler null");
    exit(8);
  }

  free(compiler);
}

#define FN_DECLARE(_label, _label_pos, _arity)                                 \
  do {                                                                         \
    compiler->fn[compiler->fn_count++] = (Fn){                                 \
        .label = _label,                                                       \
        .label_pos = _label_pos,                                               \
        .arity = arity,                                                        \
    };                                                                         \
  } while (0);
#define FN_CURR &compiler->fn[compiler->fn_count - 1]

static int RETURNED = 0;
static void compiler_stmt_fn(Compiler *compiler, Token *tokens) {
  // Pre
  MUNCH_TOKEN(Token_Fn);

  EXPECT_TOKEN(Token_Identifier);
  Token *identifier = NEXT_TOKEN;
  Sv label = (Sv){
      .str = identifier->start,
      .len = identifier->len,
  };

  // jmpa -1
  PUSH_INST(MAKE_JMPA(-1));

  uint64_t label_start_pos = LOC_INST;
  uint8_t arity = 0;

  MUNCH_TOKEN(Token_LParen);
  while (1) {
    if (PEEK_TOKEN_TYPE == Token_RParen)
      break;

    // TODO: Type checking
    Token *next = NEXT_TOKEN;
    (void)next;

    EXPECT_TOKEN(Token_Identifier);
    Token *identifier = NEXT_TOKEN;
    Sv name = (Sv){
        .len = identifier->len,
        .str = identifier->start,
    };

    LOCAL_ADD(name, compiler->depth);
    arity++;

    if (PEEK_TOKEN_TYPE == Token_RParen)
      break;

    MUNCH_TOKEN(Token_Comma);
  }
  MUNCH_TOKEN(Token_RParen);

  FN_DECLARE(label, label_start_pos, arity);

  // Mid
  compiler_stmt_block(compiler, tokens);

  // Post
  if (!RETURNED) {
    Word null = {.as_u64 = 0};
    PUSH_INST(MAKE_PUSH(null));
    PUSH_INST(MAKE_STR(REG_RAX));
    PUSH_INST(MAKE_RET);
  } else {
    RETURNED = 0;
  }

  compiler->locals_count -= arity;

  uint64_t label_end_pos = LOC_INST;
  ALTER_INST(label_start_pos - 1, MAKE_JMPA(label_end_pos));
}

static Fn *compiler_fn_resolve(Compiler *compiler, Sv *label) {
  for (size_t i = 0; i < compiler->fn_count; i++) {
    Fn *fn = &compiler->fn[i];

    if (fn->label.len == label->len) {
      if (memcmp(fn->label.str, label->str, label->len) == 0) {
        return fn;
      }
    }
  }

  fprintf(stderr, "Unknown Fn %.*s", label->len, label->str);
  exit(9);
}

static void compiler_expr_call(Compiler *compiler, Token *tokens, Sv label) {
  Compiler *new_compiler = compiler_call_new(compiler, label);

  Fn *fn = compiler_fn_resolve(compiler, &label);
  uint8_t arity = fn->arity;

  // ldr fp
  PUSH_INST(MAKE_LDR(REG_FP));

  // ldr ra
  PUSH_INST(MAKE_LDR(REG_RA));

  // mov cpsr, sp
  PUSH_INST(MAKE_PUSH((Word){.as_u64 = REG_SP}));
  PUSH_INST(MAKE_MOV(REG_CPSR));

  MUNCH_TOKEN(Token_LParen);

  while (1) {
    if (arity == 0) {
      break;
    }

    compiler_expr(compiler, tokens);

    if (--arity == 0) {
      break;
    }

    if (PEEK_TOKEN_TYPE != Token_Comma) {
      fprintf(stderr, "Expected %d arguments for Fn %.*s\n", fn->arity,
              label.len, label.str);
      exit(11);
    }
    MUNCH_TOKEN(Token_Comma);
  }

  // mov fp, cpsr
  PUSH_INST(MAKE_PUSH((Word){.as_u64 = REG_CPSR}));
  PUSH_INST(MAKE_MOV(REG_FP));

  // str ra, #offset
  uint64_t ra = LOC_INST + 3;
  PUSH_INST(MAKE_PUSH((Word){.as_u64 = ra}));
  PUSH_INST(MAKE_STR(REG_RA));

  // jmp label
  PUSH_INST(MAKE_JMPA(fn->label_pos));

  // Post call
  for (size_t i = 0; i < fn->arity; i++) {
    PUSH_INST(MAKE_POP);
  }

  compiler_call_destruct(new_compiler);

  // mov sp, fp
  PUSH_INST(MAKE_PUSH((Word){.as_u64 = REG_FP}));
  PUSH_INST(MAKE_MOV(REG_SP));

  // str ra
  PUSH_INST(MAKE_STR(REG_RA));

  // str fp
  PUSH_INST(MAKE_STR(REG_FP));

  // ldr rax
  PUSH_INST(MAKE_LDR(REG_RAX));

  MUNCH_TOKEN(Token_RParen);
}

static void compiler_stmt_return(Compiler *compiler, Token *tokens) {
  MUNCH_TOKEN(Token_Return);

  RETURNED = 1;

  if (PEEK_TOKEN_TYPE == Token_Semicolon) {
    Word null = {.as_u64 = 0};
    PUSH_INST(MAKE_PUSH(null));

  } else {
    compiler_expr(compiler, tokens);
  }

  // str rax
  PUSH_INST(MAKE_STR(REG_RAX));

  // ret
  PUSH_INST(MAKE_RET);

  MUNCH_TOKEN(Token_Semicolon);
}

static void compiler_stmt(Compiler *compiler, Token *tokens) {
  Token_t peek_type = PEEK_TOKEN_TYPE;
  Token *peek_peek = PEEK_PEEK_TOKEN;

  if (peek_type == Token_Int || peek_type == Token_Str) {
    compiler_stmt_define(compiler, tokens);

  } else if (peek_type == Token_Identifier && peek_peek->type == Token_Equal) {
    compiler_stmt_assign(compiler, tokens);

  } else if (peek_type == Token_If) {
    compiler_stmt_if(compiler, tokens);

  } else if (peek_type == Token_While) {
    compiler_stmt_while(compiler, tokens);

  } else if (peek_type == Token_Print) {
    compiler_stmt_print(compiler, tokens);

  } else if (peek_type == Token_LBrace) {
    compiler_stmt_block(compiler, tokens);

  } else if (peek_type == Token_Fn) {
    compiler_stmt_fn(compiler, tokens);

  } else if (peek_type == Token_Return) {
    compiler_stmt_return(compiler, tokens);

  } else {
    compiler_expr_stmt(compiler, tokens);
  };
}

void compiler_compile(Compiler *compiler, Token *tokens) {
  while (1) {
    if (PEEK_TOKEN_TYPE == Token_EOF)
      break;

    compiler_stmt(compiler, tokens);
  }

  PUSH_INST(MAKE_EOF);
}

#undef PEEK_TOKEN
#undef PEEK_PEEK_TOKEN
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
#undef FN_CURR
