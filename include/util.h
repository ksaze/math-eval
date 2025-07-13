#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

typedef struct substring {
  char *str;
  size_t len;
} substring;

void logError(const char *message, const char *functionName);

#endif
