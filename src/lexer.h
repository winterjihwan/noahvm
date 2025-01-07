#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
typedef enum {
  Token_LParen,
  Token_RParen,
  Token_Identifier,
  Token_Number,
  Token_Float,
  Token_LBracket,
  Token_RBracket,
  Token_LBrace,
  Token_RBrace,
  Token_Comma,
  Token_Fn,
  Token_Int,
  Token_Str,
  Token_Long,
  Token_Return,
  Token_Print,
  Token_If,
  Token_Else,
  Token_While,
  Token_For,
  Token_Plus,
  Token_Minus,
  Token_Mult,
  Token_Div,
  Token_Equal,
  Token_EqualEqual,
  Token_ExcMarkEqual,
  Token_ExcMark,
  Token_GT,
  Token_LT,
  Token_And,
  Token_Or,
  Token_Semicolon,
  Token_Colon,
  Token_EOF
} Token_t;

typedef struct {
  Token_t type;
  char *start;
  int len;
} Token;

#define TOKENS_CAP 512

typedef struct {
  char *code;
  Token tokens[TOKENS_CAP];
  size_t tokens_count;
} Lexer;

void lexer_lex(void);
void lexer_init_with_code(char *code);
void lexer_tokens_dump(Token *const tokens);
char *lexer_token_t_to_str(const Token_t type);

#endif
