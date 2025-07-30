#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

typedef struct token token;
typedef int errCodes;
enum {
  MISSING_ERROR_CODE = 1000,
  INVALID_NUMBER_FORMAT,
  INVALID_OPERATOR,
  UNDERFLOW,
  OVERFLOW,
  INVALID_OPERAND,
  MISSING_OPERATOR,
  MISSING_CLOSING_PARENTHESIS,
  UNMATCHED_CLOSING_PARENTHEIS,
  PREMATURE_END_OF_EXPRESSION,
  INVALID_ASSIGNMENT_SYNTAX,
  NESTED_ASSIGNMENT,
  UNKNOWN_IDENTIFIER,
};

void logError(const char *message, const char *functionName);
void createErrorMessage(char *buffer, size_t bufferSize, const token *tkn);

#endif
