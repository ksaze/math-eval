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
static inline bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static inline token tokenInit(lexer *lexer, tokenType type,
                              size_t tokenStartIndex, tokenErrorCode errCode) {
  return (token){
      .type = type,
      .lexeme = (substring){.str = (char *)lexer->input + tokenStartIndex,
                            .len = (size_t)(lexer->current - tokenStartIndex)},
      .pos = tokenStartIndex,
      .errCode = errCode,
  };
}

static tokenType previousTokenType;

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

static bool findEndOfLexeme(lexer *lxr, tokenType type) {
  switch (type) {
  case TOKEN_IDEN: {
    static const char *delimiters = " \r\n\t+-*/^()0123456789";
    while (lxr->input[lxr->current] != '\0' &&
           !strchr(delimiters, lxr->input[lxr->current])) {
      lxr->current++;
    }
    return true;
  }

  case TOKEN_NUMBER: {
    bool decimalPointFlag = false;
    while (is_digit(lxr->input[lxr->current]) ||
           (lxr->input[lxr->current] == '.')) {
      if (lxr->input[lxr->current] == '.') {
        if (decimalPointFlag || !is_digit(lxr->input[lxr->current + 1])) {
          // include char after '.' in the lexeme for proper error description
          lxr->current += 2;
          return false;
        }
        decimalPointFlag = true;
      }

      lxr->current++;
    }
    return true;
  }

  default:
    return 0;
  }
}

static bool UnaryOperatorContext(tokenType previousTokenType) {
  switch (previousTokenType) {
  case TOKEN_NUMBER:
  case TOKEN_IDEN:
  case TOKEN_CLOSEPAREN:
  case TOKEN_LOG:
  case TOKEN_SIN:
  case TOKEN_COS:
    return false;
  default:
    return true;
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
    *tkn = tokenInit(lxr, TOKEN_EOF, tokenStartIndex, NONE);
    return tkn;
  }

  if (is_digit(current)) {
    if (findEndOfLexeme(lxr, TOKEN_NUMBER))
      *tkn = tokenInit(lxr, TOKEN_NUMBER, tokenStartIndex, NONE);
    else
      *tkn =
          tokenInit(lxr, TOKEN_ERROR, tokenStartIndex, INVALID_NUMBER_FORMAT);
    return tkn;
  }

  lxr->current++;
  switch (current) {
  case '+':
    *tkn = tokenInit(lxr, TOKEN_PLUS, tokenStartIndex, NONE);
    break;
  case '-':
    if (UnaryOperatorContext(previousTokenType))
      *tkn = tokenInit(lxr, TOKEN_UNARY_MINUS, tokenStartIndex, NONE);
    else
      *tkn = tokenInit(lxr, TOKEN_MINUS, tokenStartIndex, NONE);
    break;
  case '*':
    *tkn = tokenInit(lxr, TOKEN_MUL, tokenStartIndex, NONE);
    break;
  case '/':
    *tkn = tokenInit(lxr, TOKEN_DIV, tokenStartIndex, NONE);
    break;
  case '^':
    *tkn = tokenInit(lxr, TOKEN_EXP, tokenStartIndex, NONE);
    break;
  case 'e':
    if (!UnaryOperatorContext(previousTokenType)) {
      *tkn = tokenInit(lxr, TOKEN_SCI_EXP, tokenStartIndex, NONE);
      break;
    }
    // If not scientific notation, e could be part of an identifer
    // Fall through to parse e as an identifier
    __attribute__((fallthrough));
  case '(':
    *tkn = tokenInit(lxr, TOKEN_OPENPAREN, tokenStartIndex, NONE);
    break;
  case ')':
    *tkn = tokenInit(lxr, TOKEN_CLOSEPAREN, tokenStartIndex, NONE);
    break;

    // handles identifiers/constants, sin, cos, and tan
  default:
    lxr->current--;
    if (!is_alpha(current)) {
      *tkn = tokenInit(lxr, TOKEN_ERROR, tokenStartIndex, INVALID_OPERATOR);
      break;
    }

    findEndOfLexeme(lxr, TOKEN_IDEN);
    if (strncmp(&lxr->input[tokenStartIndex], "log", 3) == 0)
      *tkn = tokenInit(lxr, TOKEN_LOG, tokenStartIndex, NONE);

    else if (strncmp(&lxr->input[tokenStartIndex], "sin", 3) == 0)
      *tkn = tokenInit(lxr, TOKEN_SIN, tokenStartIndex, NONE);

    else if (strncmp(&lxr->input[tokenStartIndex], "cos", 3) == 0)
      *tkn = tokenInit(lxr, TOKEN_COS, tokenStartIndex, NONE);

    else
      *tkn = tokenInit(lxr, TOKEN_IDEN, tokenStartIndex, NONE);

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
  // Global variable for context based lexing.
  // Initialised to OPENPAREN to prevent errors for the first token
  // OPENPAREN has the same rules as the start of the file
  previousTokenType = TOKEN_OPENPAREN;

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
    previousTokenType = tkn->type;
  } while (tkn->type != TOKEN_EOF && tkn->type != TOKEN_ERROR);

  if (tkn->type == TOKEN_ERROR) {
    char error_message[256];

    if (!tkn->errCode) {
      logError("Missing error code in a error token", __func__);
    }

    switch (tkn->errCode) {
    case INVALID_NUMBER_FORMAT:
      snprintf(error_message, sizeof(error_message),
               "Invalid Number Format: Incorrect placement of decimal point at "
               "%zu — '%.*s' is not a valid token\n",
               tkn->pos, (int)tkn->lexeme.len, tkn->lexeme.str);
      break;

    case INVALID_OPERATOR:
      snprintf(
          error_message, sizeof(error_message),
          "Invalid Operator: '%.*s' at index %zu is not a valid operator\n",
          (int)tkn->lexeme.len, tkn->lexeme.str, tkn->pos);
      break;
    }

    logError(error_message, __func__);

    if (freeTokenList(tokenList) == -1)
      logError("Invalid token list. No ERROR or EOF found.", __func__);
    free(lxr);
    return NULL;
  }

  // realloc for shrining is generally safe so NULL check was added
  tokenList = realloc(tokenList, sizeof(token *) * tokenCount);
  free(lxr);
  return tokenList;
}
