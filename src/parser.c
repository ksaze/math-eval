#include "parser.h"
#include "ds.h"
#include "lexer.h"
#include "util.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define GET_CURRENT_TOKEN psr->tokenStream->stream[psr->currentToken]
#define GET_TOKEN(t) psr->tokenStream->stream[t]

static precedence precedenceMap[TOKEN_MAX] = {
    [TOKEN_PLUS] = PRECEDENCE_TERM,     [TOKEN_MINUS] = PRECEDENCE_TERM,
    [TOKEN_MUL] = PRECEDENCE_FACTOR,    [TOKEN_DIV] = PRECEDENCE_FACTOR,
    [TOKEN_EXP] = PRECEDENCE_POWER,     [TOKEN_UNARY_MINUS] = PRECEDENCE_UNARY,
    [TOKEN_SIN] = PRECEDENCE_UNARY,     [TOKEN_COS] = PRECEDENCE_UNARY,
    [TOKEN_LOG] = PRECEDENCE_UNARY,     [TOKEN_OPENPAREN] = PRECEDENCE_MIN,
    [TOKEN_CLOSEPAREN] = PRECEDENCE_MIN};

static inline bool isUnary(tokenType type) {
  switch (type) {
  case TOKEN_UNARY_MINUS:
  case TOKEN_SIN:
  case TOKEN_COS:
  case TOKEN_LOG:
    return true;
  }
  return false;
}

static inline void nodeInit(parserNode *node, token tkn) {
  memset(node, 0, sizeof(parserNode));
  node->type = tkn.type;
  node->pos = tkn.pos;
}

static parserNode *parseNumber(parser *psr) {
  char *numberLexeme = GET_CURRENT_TOKEN.lexeme.str;
  char *end;

  errno = 0;
  double num = strtod(numberLexeme, &end);
  if (errno == ERANGE) {
    if (num == 0.0) {
      errno = UNDERFLOW;
      return NULL;
    } else if (num == HUGE_VAL || num == -HUGE_VAL) {
      errno = OVERFLOW;
      return NULL;
    }
  }

  parserNode *node = memPool_alloc(&psr->nodePool);
  nodeInit(node, psr->tokenStream->stream[psr->currentToken]);
  node->number = num;
  psr->currentToken++;
  return node;
}

static parserNode *parsePrefixExpression(parser *psr) {
  parserNode *ret = NULL;

  if (GET_CURRENT_TOKEN.type == TOKEN_NUMBER)
    ret = parseNumber(psr); // Returns NULL for UNDERFLOW and OVERFLOW
  else if (GET_CURRENT_TOKEN.type == TOKEN_OPENPAREN) {
    psr->currentToken++;
    ret = parseExpression(psr);

    if (GET_CURRENT_TOKEN.type == TOKEN_EOF) {
      errno = MISSING_CLOSING_PARENTHESIS;
      return NULL;
    }

    if (GET_CURRENT_TOKEN.type == TOKEN_CLOSEPAREN) {
      psr->currentToken++;
    }

  } else if (isUnary(GET_CURRENT_TOKEN.type)) {
    ret = memPool_alloc(&psr->nodePool);
    nodeInit(ret, GET_CURRENT_TOKEN);
    psr->currentToken++;
    ret->unary.operand = parsePrefixExpression(psr);
  }

  if (!ret) {
    errno = INVALID_UNARY_OPERAND;
  }

  return ret;
}

static parserNode *parseInfixExpression(parser *psr, token currentOperator,
                                        parserNode *left) {
  parserNode *ret = memPool_alloc(&psr->nodePool);
  nodeInit(ret, currentOperator);
  ret->binary.left = left;
  ret->binary.right = parseExpression(psr);
  return ret;
}

parserNode *parseExpression(parser *psr) {

  if (psr->currentToken >= psr->tokenStream->count) {
    logError("Application Failure: Tried to parse beyond EOF", __func__);
  }
  precedence previousPrecedence =
      (psr->currentToken == 0)
          ? PRECEDENCE_MIN
          : precedenceMap[GET_TOKEN(psr->currentToken - 1).type];

  parserNode *left = parsePrefixExpression(psr);
  if (left == NULL) {
    logError("Invalid syntax: Expected a unary operator or number", __func__);
  }

  token currentOperator = GET_CURRENT_TOKEN;
  precedence currentPrecedence = precedenceMap[currentOperator.type];

  while (currentPrecedence != PRECEDENCE_MIN) {
    if (previousPrecedence >= currentPrecedence)
      break;

    psr->currentToken++;
    left = parseInfixExpression(psr, currentOperator, left);
    currentOperator = GET_CURRENT_TOKEN;
    currentPrecedence = precedenceMap[currentOperator.type];
  }

  return left;
}
