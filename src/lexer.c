#include "lexer.h"
#include "util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline bool is_digit(char c) { return c >= '0' && c <= '9'; }
static inline bool is_whitespace(char c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

static inline token tokenInit(lexer *lexer, tokenType type,
                              size_t tokenStartIndex) {
  return (token){
      .type = type,
      .lexeme = (substring){.str = (char *)lexer->input + tokenStartIndex,
                            .len = (size_t)(lexer->current - tokenStartIndex)},
      .pos = tokenStartIndex,
  };
}

static lexer *lexerInit(const char *input) {
  lexer *lxr = malloc(sizeof(lexer));
  if (!lxr) {
    return NULL;
  }
  lxr->input = input;
  lxr->current = 0;
  return lxr;
}

static void skipWhiteSpace(lexer *lxr) {
  if (!lxr || !lxr->input)
    return;
  while (lxr->input[lxr->current] != '\0' &&
         is_whitespace(lxr->input[lxr->current])) {
    lxr->current++;
  }
}

static void findEndOfLexeme(lexer *lxr) {
  static const char *delimiters = " \r\n\t+-*/^()";
  while (lxr->input[lxr->current] != '\0' &&
         !strchr(delimiters, lxr->input[lxr->current])) {
    lxr->current++;
  }
}

/*
Returns TOKEN_ERROR in case of a syntax error
Returns NULL if there is a memory allocation error
*/
static token *nextToken(lexer *lxr) {
  skipWhiteSpace(lxr);

  token *tkn = malloc(sizeof(token));
  if (tkn == NULL)
    return NULL;

  char current = lxr->input[lxr->current];
  size_t tokenStartIndex = lxr->current;

  if (current == '\0') {
    *tkn = tokenInit(lxr, TOKEN_EOF, tokenStartIndex);
    return tkn;
  }

  if (is_digit(current)) {
    bool decimalPointFlag = false;
    while (is_digit(lxr->input[lxr->current]) ||
           (lxr->input[lxr->current] == '.' && !decimalPointFlag)) {
      if (lxr->input[lxr->current] == '.') {
        if (decimalPointFlag || !is_digit(lxr->input[lxr->current + 1])) {
          *tkn = tokenInit(lxr, TOKEN_ERROR, tokenStartIndex);
          return tkn;
        }
        decimalPointFlag = true;
      }

      lxr->current++;
    }

    *tkn = tokenInit(lxr, TOKEN_NUMBER, tokenStartIndex);
    return tkn;
  }

  lxr->current++;
  switch (current) {
  case '+':
    *tkn = tokenInit(lxr, TOKEN_PLUS, tokenStartIndex);
    break;
  case '-':
    *tkn = tokenInit(lxr, TOKEN_MINUS, tokenStartIndex);
    break;
  case '*':
    *tkn = tokenInit(lxr, TOKEN_MUL, tokenStartIndex);
    break;
  case '/':
    *tkn = tokenInit(lxr, TOKEN_DIV, tokenStartIndex);
    break;
  case '^':
    *tkn = tokenInit(lxr, TOKEN_EXP, tokenStartIndex);
    break;
  case '(':
    *tkn = tokenInit(lxr, TOKEN_OPENPAREN, tokenStartIndex);
    break;
  case ')':
    *tkn = tokenInit(lxr, TOKEN_CLOSEPAREN, tokenStartIndex);
    break;

  default:
    lxr->current--;
    findEndOfLexeme(lxr);
    *tkn = tokenInit(lxr, TOKEN_ERROR, tokenStartIndex);
    break;
  }

  return tkn;
}

/*
All tokenLists must end with either a TOKEN_EOF or TOKEN_ERROR.
If this is not the case, a fatal data consistency error is present.
*/
int freeTokenList(token **tokenList) {
  if (!tokenList) {
    return 0;
  }

  bool endTokenFound = false;
  for (size_t i = 0; tokenList[i]; i++) {
    if (tokenList[i]->type == TOKEN_EOF || tokenList[i]->type == TOKEN_ERROR)
      endTokenFound = true;

    free(tokenList[i]);

    if (endTokenFound)
      break;
  }
  free(tokenList);

  if (!endTokenFound) {
    return -1;
  }

  return 0;
}

token **tokenise(const char *input) {
  lexer *lxr = lexerInit(input);
  if (lxr == NULL) {
    logError("Fatal: Memory allocation failure.", __func__);
    return NULL;
  }

  size_t tokenCount = 0;
  size_t maxTokens = strlen(input) + 1;
  token **tokenList = calloc(maxTokens, sizeof(token *));
  if (tokenList == NULL) {
    logError("Fatal: Memory allocation failure.\n", __func__);
    return NULL;
  }
  token *tkn = NULL;

  do {
    tkn = nextToken(lxr);
    if (!tkn) {
      free(lxr);
      if (freeTokenList(tokenList) == -1)
        logError("Invalid token list. No ERROR or EOF found.\n", __func__);
      logError("Fatal: Memory allocation failure\n", __func__);
      return NULL;
    }

    tokenList[tokenCount++] = tkn;
  } while (tkn->type != TOKEN_EOF && tkn->type != TOKEN_ERROR);

  if (tkn->type == TOKEN_ERROR) {
    size_t error_pos = tkn->pos;
    int lexeme_len = (int)tkn->lexeme.len;
    char *lexeme_str = tkn->lexeme.str;
    if (freeTokenList(tokenList) == -1)
      logError("Invalid token list. No ERROR or EOF found.", __func__);
    free(lxr);
    char error_message[256];

    snprintf(error_message, sizeof(error_message),
             "Invalid syntax at index %zu—'%.*s' is not a valid token\n",
             error_pos, lexeme_len, lexeme_str);
    logError(error_message, __func__);
    return NULL;
  }

  // realloc for shrining is generally safe so NULL check was added
  tokenList = realloc(tokenList, sizeof(token *) * tokenCount);
  free(lxr);
  return tokenList;
}
