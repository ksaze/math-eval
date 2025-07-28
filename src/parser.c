#include "parser.h"
#include "ds.h"
#include "lexer.h"
#include "util.h"

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GET_CURRENT_TOKEN psr->tknStream->stream[psr->currentToken]
#define GET_TOKEN(t) psr->tknStream->stream[t]

int unmatchedParanthesisCount = 0;
bool parsingAssignment = false;

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

static inline void nodeInit(ASTNode *node, token tkn) {
  memset(node, 0, sizeof(ASTNode));
  node->type = tkn.type;
  node->pos = tkn.pos;
}

static ASTNode *parseNumber(parser *psr) {
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

  ASTNode *node = memPool_alloc(&psr->nodePool);
  nodeInit(node, psr->tknStream->stream[psr->currentToken]);
  node->number = num;
  psr->currentToken++;
  return node;
}

static bool assignIdentifier(parser *psr) {
  if (parsingAssignment) {
    errno = NESTED_ASSIGNMENT;
    return false;
  }
  parsingAssignment = true;
  substring key = GET_CURRENT_TOKEN.lexeme;
  psr->currentToken += 2; // skip identifier and assignment operator

  ASTNode *value = parseExpression(psr);
  if (!value)
    return false;

  hashmap_setKey(&psr->map, &key, value);
  parsingAssignment = false;
  return true;
}

static ASTNode *parsePrefixExpression(parser *psr) {
  ASTNode *ret = NULL;

  if (GET_CURRENT_TOKEN.type == TOKEN_EOF ||
      GET_CURRENT_TOKEN.type == TOKEN_CLOSEPAREN) {
    errno = PREMATURE_END_OF_EXPRESSION;
    return NULL;
  }

  if (GET_CURRENT_TOKEN.type == TOKEN_ASSIGNMENT) {
    errno = INVALID_ASSIGNMENT_SYNTAX;
    return NULL;
  }

  if (GET_CURRENT_TOKEN.type == TOKEN_NUMBER)
    ret = parseNumber(psr); // Returns NULL for UNDERFLOW and OVERFLOW
  else if (GET_CURRENT_TOKEN.type == TOKEN_OPENPAREN) {
    size_t openingParenthesisPosition = psr->currentToken;
    unmatchedParanthesisCount++;
    psr->currentToken++;

    if (GET_CURRENT_TOKEN.type == TOKEN_IDEN &&
        GET_TOKEN(psr->currentToken + 1).type == TOKEN_ASSIGNMENT) {
      if (!assignIdentifier(psr))
        return NULL;
      ret = memPool_alloc(&psr->nodePool);
      ret->type = TOKEN_ASSIGNMENT;
    }

    else {
      ret = parseExpression(psr);
      if (!ret) {
        return NULL;
      }
    }

    if (GET_CURRENT_TOKEN.type == TOKEN_EOF) {
      psr->currentToken = openingParenthesisPosition;
      errno = MISSING_CLOSING_PARENTHESIS;
      return NULL;
    }

    if (GET_CURRENT_TOKEN.type == TOKEN_CLOSEPAREN) {
      unmatchedParanthesisCount--;
      psr->currentToken++;
    }

  } else if (isUnary(GET_CURRENT_TOKEN.type) ||
             GET_CURRENT_TOKEN.type == TOKEN_PLUS ||
             GET_CURRENT_TOKEN.type == TOKEN_MINUS) {
    ret = memPool_alloc(&psr->nodePool);

    GET_CURRENT_TOKEN.type =
        (GET_CURRENT_TOKEN.type == TOKEN_PLUS)    ? TOKEN_UNARY_PLUS
        : (GET_CURRENT_TOKEN.type == TOKEN_MINUS) ? TOKEN_UNARY_MINUS
                                                  : GET_CURRENT_TOKEN.type;

    nodeInit(ret, GET_CURRENT_TOKEN);
    psr->currentToken++;
    ret->unary.operand = parsePrefixExpression(psr);
  } else {
    errno = INVALID_OPERAND;
  }

  return ret;
}

static ASTNode *parseInfixExpression(parser *psr, token currentOperator,
                                     ASTNode *left) {
  ASTNode *ret = memPool_alloc(&psr->nodePool);
  nodeInit(ret, currentOperator);
  ret->binary.left = left;
  ret->binary.right = parseExpression(psr);
  if (ret->binary.right == NULL) {
    return NULL; // error while parsing sub-expression
  }
  return ret;
}

ASTNode *parseExpression(parser *psr) {
  // Due to the function being recursive, error can be reported multiple times.
  static bool errorAlreadyReported = false;

  if (psr->currentToken >= psr->tknStream->count) {
    logError("Application Failure: Tried to parse beyond EOF", __func__);
    return NULL;
  }

  precedence previousPrecedence =
      (psr->currentToken == 0)
          ? PRECEDENCE_MIN
          : precedenceMap[GET_TOKEN(psr->currentToken - 1).type];

  ASTNode *left;

  while (true) {
    left = parsePrefixExpression(psr);

    if (errorAlreadyReported || !left || left->type != TOKEN_ASSIGNMENT)
      break;
  }

  if (errorAlreadyReported)
    return NULL;

  // Malformed unary errors
  if (!left) {

    char buffer[256];
    createErrorMessage(buffer, sizeof(buffer), &GET_CURRENT_TOKEN);
    logError(buffer, __func__);
    errorAlreadyReported = true;
    return NULL;
  }

  // Handle possible identifier declaration
  while (GET_CURRENT_TOKEN.type == TOKEN_OPENPAREN &&
         GET_TOKEN(psr->currentToken + 1).type == TOKEN_IDEN &&
         GET_TOKEN(psr->currentToken + 2).type == TOKEN_ASSIGNMENT) {
    parsePrefixExpression(psr);
  }

  token currentOperator = GET_CURRENT_TOKEN;
  precedence currentPrecedence = precedenceMap[currentOperator.type];

  // Malformed binary errors
  if (isUnary(currentOperator.type) || currentOperator.type == TOKEN_NUMBER ||
      currentOperator.type == TOKEN_IDEN ||
      currentOperator.type == TOKEN_OPENPAREN) {
    errno = MISSING_OPERATOR;
  } else if (currentOperator.type == TOKEN_CLOSEPAREN) {
    if (unmatchedParanthesisCount != 0) {
      // return back to the unary expression handling function
      return left;
    }
    errno = UNMATCHED_CLOSING_PARENTHEIS;
  } else if (currentOperator.type == TOKEN_ASSIGNMENT) {
    errno = INVALID_ASSIGNMENT_SYNTAX;
  }

  if (errno) {
    char buffer[256];
    createErrorMessage(buffer, sizeof(buffer), &GET_CURRENT_TOKEN);
    logError(buffer, __func__);
    errorAlreadyReported = true;
    return NULL;
  }

  while (currentPrecedence != PRECEDENCE_MIN) {

    if (previousPrecedence >= currentPrecedence)
      break;

    psr->currentToken++;
    left = parseInfixExpression(psr, currentOperator, left);
    if (left == NULL) {
      return NULL;
    }

    currentOperator = GET_CURRENT_TOKEN;
    currentPrecedence = precedenceMap[currentOperator.type];
  }

  return left;
}
