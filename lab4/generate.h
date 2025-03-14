#ifndef generate_h
#define generate_h

#include "token.h"

void generateFromPostfix(const vector<Token> &postfix, const map<string, int> &varMap, const map<string, int> &argMap,
                         const string &resultVar, bool isReturn = false, bool isFuncCall = false);
void generateFromFunction(const vector<Token> &tokens, const map<string, int> &varMap,
                          const map<string, int> &funcArgMap, int &i, const string &varName);
string getPosByIdentifier(string ident, const map<string, int> &varMap, const map<string, int> &funcVarMap);
void generate(const vector<Token> &tokens);

#endif