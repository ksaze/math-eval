#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

typedef struct token token;
typedef int errCodes;
enum {
  MISSING_ERROR_CODE = 1000,
  MISSING_EOF_IN_TOKENSTREAM,
  INVALID_NUMBER_FORMAT,
  INVALID_OPERATOR,
  UNDERFLOW,
  OVERFLOW,
  INVALID_UNARY_OPERAND,
  MISSING_CLOSING_PARENTHESIS,
};

typedef struct substring {
  char *str;
  size_t len;
} substring;

void logError(const char *message, const char *functionName);
void createErrorMessage(char *buffer, size_t bufferSize, const token *tkn);

#endif
