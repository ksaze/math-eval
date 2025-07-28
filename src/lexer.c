#include "lexer.h"
#include "util.h"
#include <errno.h>
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
                              const char *tokenStart) {
  return (token){
      .type = type,
      .lexeme = (substring){.str = (char *)tokenStart,
                            .len = (size_t)(lxr->current - tokenStart)},
      .pos = (size_t)(tokenStart - lxr->start),
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

static inline bool tokenMatches(const char *tokenStart, const char *current,
                                const char *target) {
  size_t len = current - tokenStart;
  size_t targetLen = strlen(target);
  return len == targetLen && strncmp(tokenStart, target, len) == 0;
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
    bool expFlag = false;
    while (true) {
      if (is_digit((*lxr->current)))
        lxr->current++;

      else if (*lxr->current == 'e' || *lxr->current == 'E') {
        if (expFlag)
          return false;

        expFlag = true;
        lxr->current++;

        if (*lxr->current == '+' || *lxr->current == '-') {
          lxr->current++;
        }

        if (!is_digit(*lxr->current))
          return false;
      }

      else if (*lxr->current == '.') {
        lxr->current++;
        if (expFlag || decimalPointFlag || !is_digit((*lxr->current)))
          return false;
        decimalPointFlag = true;
      }

      else {
        break;
      }
    }
    return true;
  }
  default:
    return true;
  }
}

// Error code is reported through tkn.errCode
static token nextToken(lexer *lxr) {
  skipWhiteSpace(lxr);

  token tkn;
  const char current = *lxr->current;
  const char *tokenStart = lxr->current;

  if (current == '\0') {
    tkn = tokenInit(lxr, TOKEN_EOF, tokenStart);
    return tkn;
  }

  if (is_digit(current)) {
    if (findEndOfLexeme(lxr, TOKEN_NUMBER))
      tkn = tokenInit(lxr, TOKEN_NUMBER, tokenStart);
    else {
      errno = INVALID_NUMBER_FORMAT;
      tkn = tokenInit(lxr, TOKEN_ERROR, tokenStart);
    }
    return tkn;
  }

  lxr->current++;
  switch (current) {
  case '+':
    tkn = tokenInit(lxr, TOKEN_PLUS, tokenStart);
    break;
  case '-':
    tkn = tokenInit(lxr, TOKEN_MINUS, tokenStart);
    break;
  case '*':
    tkn = tokenInit(lxr, TOKEN_MUL, tokenStart);
    break;
  case '/':
    tkn = tokenInit(lxr, TOKEN_DIV, tokenStart);
    break;
  case '^':
    tkn = tokenInit(lxr, TOKEN_EXP, tokenStart);
    break;
  case '(':
    tkn = tokenInit(lxr, TOKEN_OPENPAREN, tokenStart);
    break;
  case ')':
    tkn = tokenInit(lxr, TOKEN_CLOSEPAREN, tokenStart);
    break;
  case '=':
    tkn = tokenInit(lxr, TOKEN_ASSIGNMENT, tokenStart);
    break;

  // handles identifiers/constants, sin, cos, and tan
  default:
    lxr->current--; // back to start of Lexeme
    if (!is_alpha(current)) {
      lxr->current++;
      errno = INVALID_OPERATOR;
      tkn = tokenInit(lxr, TOKEN_ERROR, tokenStart);
      break;
    }

    findEndOfLexeme(lxr, TOKEN_IDEN);
    if (tokenMatches(tokenStart, lxr->current, "log"))
      tkn = tokenInit(lxr, TOKEN_LOG, tokenStart);
    else if (tokenMatches(tokenStart, lxr->current, "sin"))
      tkn = tokenInit(lxr, TOKEN_SIN, tokenStart);
    else if (tokenMatches(tokenStart, lxr->current, "cos"))
      tkn = tokenInit(lxr, TOKEN_COS, tokenStart);
    else
      tkn = tokenInit(lxr, TOKEN_IDEN, tokenStart);
    break;
  }

  return tkn;
}

tokenStream *tokenise(const char *input) {
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

    if (errno == 0) {
      errno = MISSING_ERROR_CODE;
      logError("Application failure: Missing error code in error token",
               __func__);
    }

    createErrorMessage(error_message, sizeof(error_message), &tkn);
    free(tokenList);
    logError(error_message, __func__);
    return NULL;
  }

  token *resizedList = realloc(tokenList, sizeof(token) * tokenCount);
  if (resizedList == NULL) {
    // If realloc fails, keep the original (oversized) array
    logError("Warning: Memory reallocation failure, keeping original array.\n",
             __func__);
  } else {
    tokenList = resizedList;
  }

  if (tokenList[tokenCount - 1].type != TOKEN_EOF) {
    logError("Application failure: tokenList does not end with EOF\n",
             __func__);
    free(tokenList);
    return NULL;
  }

  tokenStream *tknStream = malloc(sizeof(tokenStream));
  tknStream->stream = tokenList;
  tknStream->count = tokenCount;

  return tknStream;
}
