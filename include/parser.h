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

typedef struct parserNode {
  tokenType type;
  size_t pos;

  union {
    double number;
    struct {
      struct parserNode *operand;
    } unary;
    struct {
      struct parserNode *left;
      struct parserNode *right;
    } binary;
  };
} parserNode;

typedef struct parser {
  memPool nodePool;
  size_t currentToken;
  tokenStream *tokenStream;
} parser;

static inline parser parserInit(tokenStream *tokenStream) {
  return (parser){
      .nodePool = {0},
      .currentToken = 0,
      .tokenStream = tokenStream,
  };
}

parserNode *parseExpression(parser *psr);

#endif
