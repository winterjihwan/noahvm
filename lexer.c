#include <stdio.h>
#include <string.h>

#include "lexer.h"

static size_t token_count = 0;

inline static void lexer_lex_token_simple(Token *tokens,
                                          const Token_t token_type) {
  tokens[token_count++] = (Token){.type = token_type};
  token_count++;
}

inline static char lexer_peek(char *code) {
  if (*++code == '\0')
    return '\0';
  return *code--;
}

void lexer_lex_keyword(Token *tokens, char *code) {
  switch (*code) {
  case 'f':
    switch (*code) {}
    return;
  }
}

void lexer_lex(char *code, Token *tokens) {
  while (1) {
    const char *c = code;

    switch (*c) {
    case '[':
      lexer_lex_token_simple(tokens, Token_LBracket);
      continue;
    case ']':
      lexer_lex_token_simple(tokens, Token_RBracket);
      continue;
    case '(':
      lexer_lex_token_simple(tokens, Token_LParen);
      continue;
    case ')':
      lexer_lex_token_simple(tokens, Token_RParen);
      continue;
    }
  }
}
