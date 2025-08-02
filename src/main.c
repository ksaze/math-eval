#include "ds.h"
#include "eval.h"
#include "lexer.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t configLen = 0;

char *readConfigFile(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    // Config file is optional, so don't treat this as an error
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (fileSize <= 0) {
    logError("I/O error while processing config. Continuing without config\n",
             __func__);
    fclose(file);
    return NULL;
  }

  char *buffer = malloc(fileSize + 1);
  if (!buffer) {
    logError("Fatal: Memory allocation failure\n", __func__);
    fclose(file);
    return NULL;
  }

  size_t bytesRead = fread(buffer, 1, fileSize, file);
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

char *createCombinedInput(const char *configContent, const char *userInput) {
  if (!configContent) {
    // No config content, just duplicate user input
    size_t len = strlen(userInput);
    char *combined = malloc(len + 1);
    if (combined) {
      strcpy(combined, userInput);
    }
    return combined;
  }

  configLen = strlen(configContent);
  size_t userLen = strlen(userInput);

  char *combined = malloc(configLen + 1 + userLen + 1);
  if (!combined) {
    return NULL;
  }

  strcpy(combined, configContent);
  strcat(combined, userInput);

  return combined;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    logError("Usage: program <expression>", "main");
    return -1;
  }

  const char *userInput = argv[1];

  // Read config file
  char *configContent = readConfigFile("config.txt");
  if (errno)
    return -1;

  // Create combined input string
  char *input = createCombinedInput(configContent, userInput);
  if (!input) {
    logError("Fatal: Memory allocation failure for input string", "main");
    free(configContent);
    return -1;
  }

  // Tokenize the combined input
  tokenStream *tknStream = tokenise(input);
  if (!tknStream) {
    free(configContent);
    free(input);
    return -1;
  }

  // Initialize parser
  parser psr = {0};
  psr.currentToken = 0;
  psr.unmatchedParanthesisCount = 0;
  psr.parsingAssignment = false;
  psr.tknStream = tknStream;

  if (!memPool_init(&psr.nodePool, tknStream->count) ||
      !hashMap_init(&psr.map, (size_t)tknStream->count / 5)) {
    logError("Fatal: Memory allocation failure", "memPool_init");
    free(configContent);
    free(input);
    free(tknStream->stream);
    free(tknStream);
    return -1;
  }

  // Parse and evaluate
  ASTNode *root = parseExpression(&psr);
  if (!root) {
    free(configContent);
    free(input);
    free(tknStream->stream);
    free(tknStream);
    hashMap_free(&psr.map);
    memPool_free(&psr.nodePool);
    return -1;
  }

  double result = eval(root);
  printf("%.15g\n", result);

  // Cleanup
  free(configContent);
  free(input);
  free(tknStream->stream);
  free(tknStream);
  hashMap_free(&psr.map);
  memPool_free(&psr.nodePool);

  return 0;
}
