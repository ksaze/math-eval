#include "lexer.h"
#include "util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline bool is_digit(char c) { return c >= '0' && c <= '9'; }
static inline bool is_whitespace(char c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
static inline bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static inline token tokenInit(lexer *lxr, tokenType type,
                              const char *tokenStart, tokenErrorCode errCode) {
  return (token){
      .type = type,
      .lexeme = (substring){.str = (char *)tokenStart,
                            .len = (size_t)(lxr->current - tokenStart)},
      .pos = (size_t)(tokenStart - lxr->start),
      .errCode = errCode,
  };
}

static inline lexer lexerInit(const char *input) {
  return (lexer){
      .start = input,
      .current = input,
      .previousTokenType = TOKEN_OPENPAREN,
  };
}

static inline void skipWhiteSpace(lexer *lxr) {
  while (*lxr->current != '\0' && is_whitespace(*lxr->current)) {
    lxr->current++;
  }
}

static inline bool UnaryOperatorContext(tokenType previousTokenType) {
  switch (previousTokenType) {
  case TOKEN_NUMBER:
  case TOKEN_IDEN:
  case TOKEN_CLOSEPAREN:
  case TOKEN_LOG:
  case TOKEN_SIN:
  case TOKEN_COS:
    return false;
  default:
    return true;
  }
}

static bool findEndOfLexeme(lexer *lxr, tokenType type) {
  switch (type) {
  case TOKEN_IDEN: {
    static const char *delimiters = " \r\n\t+-*/^()0123456789";
    while (*lxr->current != '\0' && !strchr(delimiters, *lxr->current)) {
      lxr->current++;
    }
    return true;
  }

  case TOKEN_NUMBER: {
    bool decimalPointFlag = false;
    while (is_digit(*lxr->current) || (*lxr->current == '.')) {
      if (*lxr->current == '.') {
        if (decimalPointFlag || !is_digit(*(lxr->current + 1))) {
          // include char after '.' in the lexeme for proper error description
          lxr->current += 2;
          return false;
        }
        decimalPointFlag = true;
      }

      lxr->current++;
    }
    return true;
  }

  default:
    return 0;
  }
}

/*
Returns TOKEN_ERROR in case of a syntax error
Returns NULL if there is a memory allocation error
*/
static token nextToken(lexer *lxr) {
  skipWhiteSpace(lxr);

  token tkn;
  const char current = *lxr->current;
  const char *tokenStart = lxr->current;

  if (current == '\0') {
    tkn = tokenInit(lxr, TOKEN_EOF, tokenStart, NONE);
    return tkn;
  }

  if (is_digit(current)) {
    if (findEndOfLexeme(lxr, TOKEN_NUMBER))
      tkn = tokenInit(lxr, TOKEN_NUMBER, tokenStart, NONE);
    else
      tkn = tokenInit(lxr, TOKEN_ERROR, tokenStart, INVALID_NUMBER_FORMAT);
    return tkn;
  }

  lxr->current++;
  switch (current) {
  case '+':
    tkn = tokenInit(lxr, TOKEN_PLUS, tokenStart, NONE);
    break;
  case '-':
    if (UnaryOperatorContext(lxr->previousTokenType))
      tkn = tokenInit(lxr, TOKEN_UNARY_MINUS, tokenStart, NONE);
    else
      tkn = tokenInit(lxr, TOKEN_MINUS, tokenStart, NONE);
    break;
  case '*':
    tkn = tokenInit(lxr, TOKEN_MUL, tokenStart, NONE);
    break;
  case '/':
    tkn = tokenInit(lxr, TOKEN_DIV, tokenStart, NONE);
    break;
  case '^':
    tkn = tokenInit(lxr, TOKEN_EXP, tokenStart, NONE);
    break;
  case 'e':
    if (!UnaryOperatorContext(lxr->previousTokenType)) {
      tkn = tokenInit(lxr, TOKEN_SCI_EXP, tokenStart, NONE);
      break;
    }
    // If not scientific notation, e could be part of an identifer
    // Fall through to parse e as an identifier
    __attribute__((fallthrough));
  case '(':
    tkn = tokenInit(lxr, TOKEN_OPENPAREN, tokenStart, NONE);
    break;
  case ')':
    tkn = tokenInit(lxr, TOKEN_CLOSEPAREN, tokenStart, NONE);
    break;

    // handles identifiers/constants, sin, cos, and tan
  default:
    lxr->current--;
    if (!is_alpha(current)) {
      tkn = tokenInit(lxr, TOKEN_ERROR, tokenStart, INVALID_OPERATOR);
      break;
    }

    findEndOfLexeme(lxr, TOKEN_IDEN);
    if (strncmp(tokenStart, "log", 3) == 0 && lxr->current - tokenStart == 3)
      tkn = tokenInit(lxr, TOKEN_LOG, tokenStart, NONE);
    else if (strncmp(tokenStart, "sin", 3) == 0 &&
             lxr->current - tokenStart == 3)
      tkn = tokenInit(lxr, TOKEN_SIN, tokenStart, NONE);
    else if (strncmp(tokenStart, "cos", 3) == 0 &&
             lxr->current - tokenStart == 3)
      tkn = tokenInit(lxr, TOKEN_COS, tokenStart, NONE);

    else
      tkn = tokenInit(lxr, TOKEN_IDEN, tokenStart, NONE);
    break;
  }

  return tkn;
}

token *tokenise(const char *input) {
  lexer lxr = lexerInit(input);

  size_t tokenCount = 0;
  size_t maxTokens = strlen(input) + 1;

  token *tokenList = calloc(maxTokens, sizeof(token));
  if (tokenList == NULL) {
    logError("Fatal: Memory allocation failure.\n", __func__);
    return NULL;
  }

  token tkn;
  do {
    tkn = nextToken(&lxr);
    tokenList[tokenCount++] = tkn;
    lxr.previousTokenType = tkn.type;
  } while (tkn.type != TOKEN_EOF && tkn.type != TOKEN_ERROR);

  if (tkn.type == TOKEN_ERROR) {
    char error_message[256];

    if (!tkn.errCode) {
      logError("Missing error code in a error token", __func__);
    }

    switch (tkn.errCode) {
    case INVALID_NUMBER_FORMAT:
      snprintf(error_message, sizeof(error_message),
               "Invalid Number Format: Incorrect placement of decimal point at "
               "index %zu — '%.*s' is not a valid token\n",
               tkn.pos, (int)tkn.lexeme.len, tkn.lexeme.str);
      break;

    case INVALID_OPERATOR:
      snprintf(
          error_message, sizeof(error_message),
          "Invalid Operator: '%.*s' at index %zu is not a valid operator\n",
          (int)tkn.lexeme.len, tkn.lexeme.str, tkn.pos);
      break;
    }

    free(tokenList);
    logError(error_message, __func__);
  }

  // realloc for shrining is generally safe so NULL check was added
  tokenList = realloc(tokenList, sizeof(token) * tokenCount);

  if (tokenList[tokenCount - 1].type != TOKEN_EOF) {
    logError("Application failure: tokenList does not end with EOF\n",
             __func__);
    free(tokenList);
    return NULL;
  }

  return tokenList;
}
