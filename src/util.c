#include "util.h"
#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

extern size_t configLen;

void logError(const char *message, const char *funcName) {
  if (errno >= 1000) {
    fprintf(stderr, "%s", message);
  } else
    perror(message);

  FILE *logfile = fopen("log.txt", "a");
  if (logfile == NULL) {
    fprintf(stderr, "Failed to open log file\n");
    return;
  }

  time_t now;
  struct tm *timeinfo;
  char time_str[20];

  time(&now);
  timeinfo = localtime(&now);
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

  fprintf(logfile, "--------------------------------------------------\n");
  fprintf(logfile, "Timestamp: %s\n", time_str);
  fprintf(logfile, "Function: %s\n", funcName);
  fprintf(logfile, "Error Message: %s\n", message);
  if (errno < 1000)
    fprintf(logfile, "System Error: %s\n", strerror(errno));
  else {
    fprintf(logfile, "Application error: %s\n", message);
  }

  fprintf(logfile, "--------------------------------------------------\n\n");

  fclose(logfile);
}

void createErrorMessage(char *buffer, size_t bufferSize, const token *tkn) {
  switch (errno) {
  case INVALID_NUMBER_FORMAT:
    snprintf(buffer, bufferSize,
             "Invalid Number Format: Incorrect placement of decimal point at "
             "index %zu — '%.*s' is not a valid token\n",
             tkn->pos - configLen, (int)tkn->lexeme.len, tkn->lexeme.str);
    break;

  case INVALID_OPERATOR:
    snprintf(buffer, bufferSize,
             "Invalid Operator: '%.*s' at index %zu is not a valid operator\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos - configLen);
    break;

  case OVERFLOW:
    snprintf(buffer, bufferSize,
             "Overflow: Failed to evaluate '%.*s' at position %zu\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos - configLen);
    break;

  case UNDERFLOW:
    snprintf(buffer, bufferSize,
             "Overflow: Failed to evaluate '%.*s' at position %zu.\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos - configLen);
    break;

  case INVALID_OPERAND:
    snprintf(buffer, bufferSize,
             "Invalid Operand: Failed to parse '%.*s' at position %zu. Can't "
             "pass a binary operator as an operand.\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos - configLen);
    break;

  case MISSING_OPERATOR:
    snprintf(buffer, bufferSize,
             "Missing Operator: Expected a binary operator before '%.*s' at "
             "position %zu.\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos - configLen);
    break;

  case MISSING_CLOSING_PARENTHESIS:
    snprintf(buffer, bufferSize,
             "Missing Paranthesis: Failed to find a matching closing "
             "parenthesis for the parenthesis at position %zu\n",
             tkn->pos - configLen);
    break;

  case UNMATCHED_CLOSING_PARENTHEIS:
    snprintf(buffer, bufferSize,
             "Missing Paranthesis: Closing paranthesis without a matching "
             "opening paranthesis at position %zu\n",
             tkn->pos - configLen);
    break;

  case PREMATURE_END_OF_EXPRESSION:
    snprintf(buffer, bufferSize,
             "Missing Operand: Sub-expression ended at index %zu without "
             "resolving required operand\n",
             tkn->pos - configLen);
    break;
  case MISSING_EXPRESSION:
    snprintf(buffer, bufferSize,
             "Missing Sub-expression: Expression ended at %zu without "
             "providing any evaluable content.\n",
             tkn->pos - configLen);
    break;
  case INVALID_ASSIGNMENT_SYNTAX:
    snprintf(buffer, bufferSize,
             "Invalid Syntax: Invalid use of assignment operator at position "
             "%zu. Ensure that all identifier declarations are of the form "
             "(<iden> = <exp>)\n",
             tkn->pos - configLen);
    break;

  case NESTED_ASSIGNMENT:
    snprintf(
        buffer, bufferSize,
        "Nested Assignment: Invalid use of assignment operator at position "
        "%zu. Can't declare identifiers inside an identifier definition.\n",
        tkn->pos - configLen);
    break;
  case UNKNOWN_IDENTIFIER:
    snprintf(buffer, bufferSize,
             "Unknown Identifier: Found no definition for identifier '%.*s' at "
             "position %zu.\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos - configLen);
    break;
  case MAXIMUM_RECURSION_DEPTH:
    snprintf(buffer, bufferSize,
             "Maximum Recursion Depth: Reached maximum recursion depth while "
             "evaluating identifier '%.*s' at "
             "position %zu.\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos - configLen);
    break;
  default:
    printf("Error code: %d", errno);
    snprintf(buffer, bufferSize,
             "Found No Error Code: Application stopped while processing '%.*s' "
             "at index %zu\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos - configLen);
    break;
  }
}
