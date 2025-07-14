#include "lexer.h"
#include <criterion/criterion.h>
#include <criterion/internal/test.h>
#include <criterion/logging.h>
#include <criterion/redirect.h>

// Redirect stdout and stderr to /dev/null
void redirect_all_output(void) {
  cr_redirect_stdout();
  cr_redirect_stderr();
}

// Test suite for basic tokenization
TestSuite(lexer_basic, .description = "Basic lexer tokenization tests");

Test(lexer_basic, test_number_tokens, .init = redirect_all_output) {
  token **tokens = tokenise("123");

  cr_assert_not_null(tokens, "Tokenization should not return NULL");
  cr_assert_eq(tokens[0]->type, TOKEN_NUMBER, "First token should be NUMBER");
  cr_assert_eq(tokens[0]->lexeme.len, 3, "Token length should be 3");
  cr_assert_eq(strncmp(tokens[0]->lexeme.str, "123", 3), 0,
               "Token lexeme should be '123'");
  cr_assert_eq(tokens[0]->pos, 0, "Token position should be 0");
  cr_assert_eq(tokens[1]->type, TOKEN_EOF, "Second token should be EOF");

  freeTokenList(tokens);
}

Test(lexer_basic, test_decimal_numbers, .init = redirect_all_output) {
  token **tokens = tokenise("3.14");

  cr_assert_not_null(tokens);
  cr_assert_eq(tokens[0]->type, TOKEN_NUMBER);
  cr_assert_eq(tokens[0]->lexeme.len, 4);
  cr_assert_eq(strncmp(tokens[0]->lexeme.str, "3.14", 4), 0);
  cr_assert_eq(tokens[1]->type, TOKEN_EOF);

  freeTokenList(tokens);
}

Test(lexer_basic, test_operators, .init = redirect_all_output) {
  token **tokens = tokenise("+-*/^()");

  cr_assert_not_null(tokens);

  tokenType expected[] = {TOKEN_PLUS,       TOKEN_MINUS, TOKEN_MUL,
                          TOKEN_DIV,        TOKEN_EXP,   TOKEN_OPENPAREN,
                          TOKEN_CLOSEPAREN, TOKEN_EOF};

  for (int i = 0; i < 8; i++) {
    cr_assert_eq(tokens[i]->type, expected[i],
                 "Token %d should be type %d, got %d", i, expected[i],
                 tokens[i]->type);
    cr_assert_eq(tokens[i]->pos, i, "Token %d should be at position %d", i, i);
  }

  freeTokenList(tokens);
}

// Test suite for error handling
TestSuite(lexer_errors, .description = "Error handling tests",
          .init = redirect_all_output);

Test(lexer_errors, test_invalid_decimal, .init = redirect_all_output) {
  token **tokens = tokenise("3.14.5");
  cr_assert_null(tokens, "Invalid decimal should return NULL");
}

Test(lexer_errors, test_trailing_decimal, .init = redirect_all_output) {
  token **tokens = tokenise("3.");
  cr_assert_null(tokens, "Trailing decimal should return NULL");
}

Test(lexer_errors, test_invalid_characters, .init = redirect_all_output) {
  token **tokens = tokenise("123 @ 456");
  cr_assert_null(tokens, "Invalid characters should return NULL");
}

// Test suite for whitespace handling
TestSuite(lexer_whitespace, .description = "Whitespace handling tests");

Test(lexer_whitespace, test_whitespace_skipping, .init = redirect_all_output) {
  token **tokens = tokenise("  123   +   456  ");

  cr_assert_not_null(tokens);
  cr_assert_eq(tokens[0]->type, TOKEN_NUMBER);
  cr_assert_eq(tokens[0]->pos, 2, "First token should be at position 2");
  cr_assert_eq(tokens[1]->type, TOKEN_PLUS);
  cr_assert_eq(tokens[1]->pos, 8, "Plus token should be at position 8");
  cr_assert_eq(tokens[2]->type, TOKEN_NUMBER);
  cr_assert_eq(tokens[2]->pos, 12, "Second number should be at position 12");

  freeTokenList(tokens);
}

Test(lexer_whitespace, test_different_whitespace_types,
     .init = redirect_all_output) {
  token **tokens = tokenise("1\t+\n2\r*\n3");

  cr_assert_not_null(tokens);

  tokenType expected[] = {TOKEN_NUMBER, TOKEN_PLUS,   TOKEN_NUMBER,
                          TOKEN_MUL,    TOKEN_NUMBER, TOKEN_EOF};

  for (int i = 0; i < 6; i++) {
    cr_assert_eq(tokens[i]->type, expected[i]);
  }

  freeTokenList(tokens);
}

Test(lexer_whitespace, test_empty_input, .init = redirect_all_output) {
  token **tokens = tokenise("");

  cr_assert_not_null(tokens);
  cr_assert_eq(tokens[0]->type, TOKEN_EOF);
  cr_assert_eq(tokens[0]->pos, 0);

  freeTokenList(tokens);
}

Test(lexer_whitespace, test_only_whitespace, .init = redirect_all_output) {
  token **tokens = tokenise("   \t\n\r  ");

  cr_assert_not_null(tokens);
  cr_assert_eq(tokens[0]->type, TOKEN_EOF);

  freeTokenList(tokens);
}

// Test suite for complex expressions
TestSuite(lexer_complex, .description = "Complex expression tests");

Test(lexer_complex, test_complex_expression, .init = redirect_all_output) {
  token **tokens = tokenise("(3.14 + 2.71) * 42 / 7 - 1");

  cr_assert_not_null(tokens);

  tokenType expected[] = {TOKEN_OPENPAREN, TOKEN_NUMBER,     TOKEN_PLUS,
                          TOKEN_NUMBER,    TOKEN_CLOSEPAREN, TOKEN_MUL,
                          TOKEN_NUMBER,    TOKEN_DIV,        TOKEN_NUMBER,
                          TOKEN_MINUS,     TOKEN_NUMBER,     TOKEN_EOF};

  int expected_count = sizeof(expected) / sizeof(expected[0]);

  for (int i = 0; i < expected_count; i++) {
    cr_assert_eq(tokens[i]->type, expected[i], "Token %d: expected %d, got %d",
                 i, expected[i], tokens[i]->type);
  }

  freeTokenList(tokens);
}

Test(lexer_complex, test_consecutive_operators, .init = redirect_all_output) {
  token **tokens = tokenise("++--");

  cr_assert_not_null(tokens);
  cr_assert_eq(tokens[0]->type, TOKEN_PLUS);
  cr_assert_eq(tokens[1]->type, TOKEN_PLUS);
  cr_assert_eq(tokens[2]->type, TOKEN_MINUS);
  cr_assert_eq(tokens[3]->type, TOKEN_MINUS);
  cr_assert_eq(tokens[4]->type, TOKEN_EOF);

  freeTokenList(tokens);
}

// Test suite for edge cases
TestSuite(lexer_edge_cases, .description = "Edge case tests");

Test(lexer_edge_cases, test_single_digit, .init = redirect_all_output) {
  token **tokens = tokenise("0");

  cr_assert_not_null(tokens);
  cr_assert_eq(tokens[0]->type, TOKEN_NUMBER);
  cr_assert_eq(tokens[0]->lexeme.len, 1);
  cr_assert_eq(tokens[1]->type, TOKEN_EOF);

  freeTokenList(tokens);
}

Test(lexer_edge_cases, test_zero_decimal, .init = redirect_all_output) {
  token **tokens = tokenise("0.0");

  cr_assert_not_null(tokens);
  cr_assert_eq(tokens[0]->type, TOKEN_NUMBER);
  cr_assert_eq(tokens[0]->lexeme.len, 3);
  cr_assert_eq(tokens[1]->type, TOKEN_EOF);

  freeTokenList(tokens);
}

Test(lexer_edge_cases, test_long_number, .init = redirect_all_output) {
  token **tokens = tokenise("123456789");

  cr_assert_not_null(tokens);
  cr_assert_eq(tokens[0]->type, TOKEN_NUMBER);
  cr_assert_eq(tokens[0]->lexeme.len, 9);
  cr_assert_eq(tokens[1]->type, TOKEN_EOF);

  freeTokenList(tokens);
}

// Test with fixtures for setup/teardown
static token **test_tokens;

void setup_complex_tokens(void) { test_tokens = tokenise("1 + 2 * 3"); }

void teardown_complex_tokens(void) {
  if (test_tokens) {
    freeTokenList(test_tokens);
    test_tokens = NULL;
  }
}

Test(lexer_complex, test_with_fixture, .init = setup_complex_tokens,
     .fini = teardown_complex_tokens) {
  cr_assert_not_null(test_tokens);
  cr_assert_eq(test_tokens[0]->type, TOKEN_NUMBER);
  cr_assert_eq(test_tokens[1]->type, TOKEN_PLUS);
  cr_assert_eq(test_tokens[2]->type, TOKEN_NUMBER);
  cr_assert_eq(test_tokens[3]->type, TOKEN_MUL);
  cr_assert_eq(test_tokens[4]->type, TOKEN_NUMBER);
  cr_assert_eq(test_tokens[5]->type, TOKEN_EOF);
}
