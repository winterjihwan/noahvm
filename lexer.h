typedef enum {
  Token_LParen,
  Token_RParen,
  Token_Identifier,
  Token_Number,
  Token_LBracket,
  Token_RBracket,
  Token_LBrace,
  Token_RBrace,
  Token_Comma,
  Token_Fn,
  Token_Int,
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
  Token_ExcMark,
  Token_GT,
  Token_LT,
  Token_And,
  Token_Or,
  Token_Semicolon,
  Token_Colon
} Token_t;

typedef struct {
  Token_t type;
  char *start;
  int len;
} Token;

#define MAX_TOKENS 512

typedef struct {
  char *code;
  Token tokens[MAX_TOKENS];
} Lexer;

void lexer_lex(void);
void lexer_init_code(char *code);
void lexer_tokens_dump(Token *tokens);
