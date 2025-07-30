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
  substring identifer;

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
  int unmatchedParanthesisCount;
  int recursionDepth;
  bool parsingAssignment;
  tokenStream *tknStream;
  hashMap map;
} parser;

ASTNode *parseExpression(parser *psr);

#endif
