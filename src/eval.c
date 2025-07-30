#include "lexer.h"
#include "parser.h"
#include <math.h>

double eval(ASTNode *root) {
  switch (root->type) {
  case TOKEN_NUMBER:
    return root->number;
  case TOKEN_UNARY_MINUS:
    return -eval(root->unary.operand);
  case (TOKEN_UNARY_PLUS):
    return eval(root->unary.operand);
  case TOKEN_PLUS:
    return eval(root->binary.left) + eval(root->binary.right);
  case TOKEN_MINUS:
    return eval(root->binary.left) - eval(root->binary.right);
  case TOKEN_MUL:
    return eval(root->binary.left) * eval(root->binary.right);
  case TOKEN_DIV:
    return eval(root->binary.left) / eval(root->binary.right);
  case TOKEN_EXP:
    return pow(eval(root->binary.left), eval(root->binary.right));
  case TOKEN_SIN:
    return sin(eval(root->unary.operand));
  case TOKEN_COS:
    return cos(eval(root->unary.operand));
  case TOKEN_LOG:
    return log10(eval(root->unary.operand));
  case TOKEN_IDEN:
    return eval(root->unary.operand);
  default:
    return nan("Invalid Token");
  }
}
