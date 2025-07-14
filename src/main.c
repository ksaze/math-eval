#include "lexer.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <expression>\n", argv[0]);
    return -1;
  }

  const char *input = argv[1];
  token **tokenList = tokenise(input);
  if (!tokenList) {
    return -1;
  }

  printf("Succesful");
  freeTokenList(tokenList);
  return 0;
}
