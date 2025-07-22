#include "util.h"
#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
             tkn->pos, (int)tkn->lexeme.len, tkn->lexeme.str);
    break;

  case INVALID_OPERATOR:
    snprintf(buffer, bufferSize,
             "Invalid Operator: '%.*s' at index %zu is not a valid operator\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos);
    break;

  default:
    snprintf(buffer, bufferSize,
             "Invalid syntax: '%.*s' at index %zu is not a valid token\n",
             (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos);
    break;
  }
}
