#ifndef POSTFIX_H
#define POSTFIX_H

#include <vector>
#include "token.h"

bool isOperator(TokenType type);
int getPrecedence(TokenType type);
vector<Token> infixToPostfix(const vector<Token> &infix);

#endif