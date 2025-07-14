#ifndef LEXER_H
#define LEXER_H

#include "util.h"
#include <stddef.h>

typedef int tokenType;
enum {
  TOKEN_EOF,
  TOKEN_ERROR,

  TOKEN_NUMBER,

  TOKEN_UNARY_MINUS,

  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_MUL,
  TOKEN_DIV,
  TOKEN_EXP,
  TOKEN_SCI_EXP,

  TOKEN_OPENPAREN,
  TOKEN_CLOSEPAREN,

  TOKEN_IDEN,

  TOKEN_SIN,
  TOKEN_COS,
  TOKEN_LOG,
};

typedef int tokenErrorCode;
enum {
  NONE,
  INVALID_NUMBER_FORMAT,
  INVALID_OPERATOR,
};

typedef struct token {
  tokenType type;
  substring lexeme;
  size_t pos;
  tokenErrorCode errCode;
} token;

typedef struct lexer {
  const char *input;
  size_t current;
} lexer;

int freeTokenList(token **tokens);
token **tokenise(const char *input);

#endif
