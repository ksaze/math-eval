#ifndef LEXER_H
#define LEXER_H

#include "ds.h"
#include "util.h"
#include <stddef.h>

typedef int tokenType;
enum {
  TOKEN_EOF,
  TOKEN_ERROR,

  TOKEN_NUMBER,

  TOKEN_UNARY_MINUS,
  TOKEN_UNARY_PLUS,

  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_MUL,
  TOKEN_DIV,
  TOKEN_EXP,

  TOKEN_OPENPAREN,
  TOKEN_CLOSEPAREN,

  TOKEN_IDEN,

  TOKEN_SIN,
  TOKEN_COS,
  TOKEN_LOG,
  TOKEN_INDICATOR,

  TOKEN_ASSIGNMENT,

  TOKEN_MAX,
};

typedef struct token {
  tokenType type;
  substring lexeme;
  size_t pos;
} token;

typedef struct tokenStream {
  token *stream;
  size_t count;
} tokenStream;

typedef struct lexer {
  const char *const start;
  const char *current;
  tokenType previousTokenType;
} lexer;

tokenStream *tokenise(const char *input);

#endif
