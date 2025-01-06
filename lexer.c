#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

Lexer lexer = {0};

inline static void lexer_code_advance(const int n) {
  for (int i = 0; i < n; i++) {
    lexer.code++;
  }
}

inline static void lexer_lex_token(const Token_t token_type, const int len) {
  lexer.tokens[lexer.tokens_count++] =
      (Token){.type = token_type, .start = lexer.code, .len = len};
  lexer_code_advance(len);
}

void lexer_init_with_code(char *code) { lexer.code = code; }

static int lexer_lex_keyword(void) {
  const char *code = lexer.code;

  switch (*code) {
  case 'f':
    ++code;
    switch (*code) {
    case 'o':
      lexer_lex_token(Token_For, 3);
      return 1;
    case 'n':
      lexer_lex_token(Token_Fn, 2);
      return 1;
    default:
      return 0;
    }
    return 0;

  case 'i':
    ++code;
    switch (*code--) {
    case 'f':
      lexer_lex_token(Token_If, 2);
      return 1;
    case 'n':
      lexer_lex_token(Token_Int, 3);
      return 1;
    default:
      return 0;
    }

  case 'w':
    lexer_lex_token(Token_While, 5);
    return 1;
  case 'l':
    lexer_lex_token(Token_Long, 4);
    return 1;
  case 'p':
    lexer_lex_token(Token_Print, 5);
    return 1;
  case 'e':
    lexer_lex_token(Token_Else, 5);
    return 1;
  case 'r':
    lexer_lex_token(Token_Return, 6);
    return 1;
  default:
    return 0;
  }
  return 0;
}

inline static int is_alphabet(const int c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
inline static int is_number(const char c) { return c >= 48 && c <= 57; }

static void lexer_lex_alphabet(void) {
  int len = 1;
  char *p_code = lexer.code;
  p_code++;

  while (1) {
    char c = *p_code;

    if (c == '\0' || !is_alphabet(c)) {
      break;
    }

    len++;
    p_code++;
  }

  lexer_lex_token(Token_Identifier, len);
}

static void lexer_lex_number(void) {
  int len = 1;
  char *p_code = lexer.code;
  p_code++;

  while (1) {
    char c = *p_code;

    if (c == '.') {
      goto token_float;
    }

    if (c == '\0' || !is_number(c)) {
      break;
    }

    len++;
    p_code++;
  }

  lexer_lex_token(Token_Number, len);
  return;

token_float:
  len++;
  p_code++;
  char c = *p_code;

  if (!is_number(c)) {
    fprintf(stderr, "Expected numeric after '.'");
  }

  while (1) {
    char c = *p_code;

    if (c == '\0' || !is_number(c)) {
      break;
    }

    len++;
    p_code++;
  }

  lexer_lex_token(Token_Float, len);
}

static int lexer_lex_alphanumeric(void) {
  char c = *lexer.code;

  if (c == '_' || is_alphabet(c)) {
    lexer_lex_alphabet();
    return 1;
  } else if (is_number(c)) {
    lexer_lex_number();
    return 1;
  }

  return 0;
}

inline static void lexer_trim_space(void) {
  while (1) {
    const char c = *lexer.code;
    switch (c) {
    case ' ':
      lexer.code++;
      continue;
    case '\n':
      lexer.code++;
      continue;
    case '\t':
      lexer.code++;
      continue;
    default:
      return;
    }
  }
}

void lexer_lex(void) {
  while (1) {
    lexer_trim_space();
    const char *code = lexer.code;
    if (*code == '\0') {
      lexer_lex_token(Token_EOF, 0);
      break;
    }

    if (lexer_lex_keyword()) {
      continue;
    }

    if (lexer_lex_alphanumeric()) {
      continue;
    }

    switch (*code) {
    case '[':
      lexer_lex_token(Token_LBracket, 1);
      continue;
    case ']':
      lexer_lex_token(Token_RBracket, 1);
      continue;
    case '(':
      lexer_lex_token(Token_LParen, 1);
      continue;
    case ')':
      lexer_lex_token(Token_RParen, 1);
      continue;
    case '{':
      lexer_lex_token(Token_LBrace, 1);
      continue;
    case '}':
      lexer_lex_token(Token_RBrace, 1);
      continue;
    case ',':
      lexer_lex_token(Token_Comma, 1);
      continue;
    case '+':
      lexer_lex_token(Token_Plus, 1);
      continue;
    case '-':
      lexer_lex_token(Token_Minus, 1);
      continue;
    case '*':
      lexer_lex_token(Token_Mult, 1);
      continue;
    case '/':
      lexer_lex_token(Token_Div, 1);
      continue;
    case '=':
      if (*++code == '=') {
        lexer_lex_token(Token_EqualEqual, 2);
      } else {
        lexer_lex_token(Token_Equal, 1);
      }
      continue;
    case '!':
      lexer_lex_token(Token_ExcMark, 1);
      continue;
    case '>':
      lexer_lex_token(Token_GT, 1);
      continue;
    case '<':
      lexer_lex_token(Token_LT, 1);
      continue;
    case ';':
      lexer_lex_token(Token_Semicolon, 1);
      continue;
    case ':':
      lexer_lex_token(Token_Colon, 1);
      continue;
    default:
      fprintf(stderr, "ERROR: Unidentifiable character: %c\n", *code);
      exit(2);
    }
  }
}

char *lexer_token_t_to_str(const Token_t type) {
  switch (type) {
  case Token_LParen:
    return "Token_LParen";
  case Token_RParen:
    return "Token_RParen";
  case Token_Identifier:
    return "Token_Identifier";
  case Token_Number:
    return "Token_Number";
  case Token_Float:
    return "Token_Float";
  case Token_LBracket:
    return "Token_LBracket";
  case Token_RBracket:
    return "Token_RBracket";
  case Token_Comma:
    return "Token_Comma";
  case Token_Fn:
    return "Token_Fn";
  case Token_Int:
    return "Token_Int";
  case Token_Str:
    return "Token_Str";
  case Token_Long:
    return "Token_Long";
  case Token_Return:
    return "Token_Return";
  case Token_Print:
    return "Token_Print";
  case Token_If:
    return "Token_If";
  case Token_Else:
    return "Token_Else";
  case Token_While:
    return "Token_While";
  case Token_For:
    return "Token_For";
  case Token_Plus:
    return "Token_Plus";
  case Token_Minus:
    return "Token_Minus";
  case Token_Mult:
    return "Token_Mult";
  case Token_Div:
    return "Token_Div";
  case Token_Equal:
    return "Token_Equal";
  case Token_EqualEqual:
    return "Token_EqualEqual";
  case Token_ExcMark:
    return "Token_ExcMark";
  case Token_GT:
    return "Token_GT";
  case Token_LT:
    return "Token_LT";
  case Token_And:
    return "Token_And";
  case Token_Or:
    return "Token_Or";
  case Token_Semicolon:
    return "Token_Semicolon";
  case Token_Colon:
    return "Token_Colon";
  case Token_LBrace:
    return "Token_LBrace";
  case Token_RBrace:
    return "Token_RBrace";
  case Token_EOF:
    return "Token_EOF";
  default:
    fprintf(stderr, "ERROR: Unidentifiable token type: %d\n", type);
    exit(3);
  }
}

void lexer_tokens_dump(Token *const tokens) {
  for (size_t i = 0; i < lexer.tokens_count; i++) {
    Token *token = &tokens[i];
    printf("Type: %s\n", lexer_token_t_to_str(token->type));
    printf("Len: %d\n", token->len);
    printf("Data: %.*s\n\n", token->len, token->start);
  }
}
