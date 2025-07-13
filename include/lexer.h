#ifndef LEXER_H
#define LEXER_H

#include "util.h"
#include <stddef.h>

typedef int tokenType;
enum {
  TOKEN_EOF,
  TOKEN_ERROR,

  TOKEN_NUMBER,

  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_MUL,
  TOKEN_DIV,
  TOKEN_EXP,

  TOKEN_OPENPAREN,
  TOKEN_CLOSEPAREN,
};

typedef struct token {
  tokenType type;
  substring lexeme;
  size_t pos;
} token;

typedef struct lexer {
  const char *input;
  size_t current;
} lexer;

int freeTokenList(token **tokens);
token **tokenise(const char *input);

#endif
