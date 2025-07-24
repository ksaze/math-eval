#ifndef PARSE_H
#define PARSE_H
#include "ds.h"
#include "lexer.h"
#include <stdbool.h>

typedef int precedence;
enum {
  PRECEDENCE_MIN,
  PRECEDENCE_TERM,
  PRECEDENCE_FACTOR,
  PRECEDENCE_POWER,
  PRECEDENCE_UNARY,
  PRECEDENCE_MAX,
};

typedef struct ASTNode {
  tokenType type;
  size_t pos;

  union {
    double number;
    struct {
      struct ASTNode *operand;
    } unary;
    struct {
      struct ASTNode *left;
      struct ASTNode *right;
    } binary;
  };
} ASTNode;

typedef struct parser {
  memPool nodePool;
  size_t currentToken;
  tokenStream *tknStream;
} parser;

ASTNode *parseExpression(parser *psr);

#endif
