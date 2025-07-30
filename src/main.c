#include "ds.h"
#include "eval.h"
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

  parser psr = {0};
  psr.currentToken = 0;
  psr.unmatchedParanthesisCount = 0;
  psr.parsingAssignment = false;
  psr.tknStream = tknStream;

  if (!memPool_init(&psr.nodePool, tknStream->count) ||
      !hashMap_init(&psr.map, (size_t)tknStream->count / 5)) {
    logError("Fatal: Memory allocation failure", "memPool_init");
    return -1;
  }

  ASTNode *root = parseExpression(&psr);
  if (!root) {
    free(tknStream->stream);
    free(tknStream);
    hashMap_free(&psr.map);
    memPool_free(&psr.nodePool);
    return -1;
  }

  double result = eval(root);
  printf("%.15g\n", result);
  // printf("%.15g\n", psr.nodePool.used / (double)psr.nodePool.capacity);

  free(tknStream->stream);
  free(tknStream);
  hashMap_free(&psr.map);
  memPool_free(&psr.nodePool);
  return 0;
}
