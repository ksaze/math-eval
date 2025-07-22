#include "ds.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <expression>\n", argv[0]);
    return -1;
  }

  const char *input = argv[1];
  tokenStream *tknStream = tokenise(input);
  if (!tknStream) {
    return -1;
  }

  parser psr = parserInit(tknStream);
  if (!memPool_init(&psr.nodePool, tknStream->count)) {
    logError("Fatal: Memory allocation failure", "memPool_init");
    return -1;
  }

  parserNode *root = parseExpression(&psr);

  free(tknStream->stream);
  free(tknStream);
  memPool_free(&psr.nodePool);

  printf("Succesful");
  return 0;
}
