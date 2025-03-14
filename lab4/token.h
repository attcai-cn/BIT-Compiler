#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <regex>
#include <stack>
#include <vector>
#include <string>


using namespace std;

enum TokenType
{
    UNKNOWN=15,
    COMMA,
    MAIN_KEYWORD,
    VOID_KEYWORD,
    PRINTLN_INT,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    RETURN_KEYWORD,
    INT_KEYWORD,
    IDENTIFIER,
    ASSIGN_OP,
    INTEGER_LITERAL,
    PLUS_OP,
    MINUS_OP,
    MULTIPLY_OP,
    DIVIDE_OP,
    MODULO_OP,
    AND_OP,
    OR_OP,
    XOR_OP,
    LOGICAL_NOT,
    LOGICAL_AND,
    LOGICAL_OR,
    SEMICOLON,
    LESS_THAN,
    LESS_THAN_EQUAL,
    GREATER_THAN,
    GREATER_THAN_EQUAL,
    EQUAL,
    NOT_EQUAL,
    BITWISE_NOT, //按位非
    UNARY_MINUS,
    FUNC_CALL, //函数调用
    IF_KEYWORD,
    ELSE_KEYWORD,
    WHILE_KEYWORD,
    BREAK_KEYWORD,
    CONTINUE_KEYWORD,

};


struct Token
{
    TokenType type;
    string value;
};

vector<Token> tokenizer(const string &src);

#endif