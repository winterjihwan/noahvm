typedef enum {
  Token_LParen,
  Token_RParen,
  Token_Identifier,
  Token_Function,
  Token_Number,
  Token_LBracket,
  Token_RBracket,
  Token_Comma,
  Token_Fn,
  Token_Int,
  Token_Long,
  Token_Return,
  Token_Print,
  Token_If,
  Token_Else,
  Token_While,
  Token_For
} Token_t;

typedef struct {
  Token_t type;
} Token;

void lexer_lex(char *code, Token *tokens);
