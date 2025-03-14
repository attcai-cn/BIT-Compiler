%{
#include "token.h"
#include <vector>
#include <string>
#include "postfix.h"

using namespace std;

vector<Token> tokens;

void addToken(TokenType type, const string &value) {
    tokens.push_back({type, value});
}
%}

%option noyywrap

%x COMMENT

%%
"main"             { addToken(MAIN_KEYWORD, yytext); }
"void"             { addToken(VOID_KEYWORD, yytext); }
"println_int"      { addToken(PRINTLN_INT, yytext); }
"return"           { addToken(RETURN_KEYWORD, yytext); }
"int"              { addToken(INT_KEYWORD, yytext); }
"("                { addToken(LEFT_PAREN, yytext); }
")"                { addToken(RIGHT_PAREN, yytext); }
"{"                { addToken(LEFT_BRACE, yytext); }
"}"                { addToken(RIGHT_BRACE, yytext); }
"="                { addToken(ASSIGN_OP, yytext); }
"+"                { addToken(PLUS_OP, yytext); }
"-"                { 
                    if(tokens.back().type == TokenType::LEFT_PAREN||tokens.back().type == TokenType::ASSIGN_OP ||
                       tokens.back().type == TokenType::COMMA||isOperator(tokens.back().type))
                        {
                            addToken(UNARY_MINUS, yytext);

                        }
                        else
                        {
                            addToken(MINUS_OP, yytext);
                        } 
                    }
"*"                { addToken(MULTIPLY_OP, yytext); }
"/"                { addToken(DIVIDE_OP, yytext); }
"%"                { addToken(MODULO_OP, yytext); }
"&"                { addToken(AND_OP, yytext); }
"|"                { addToken(OR_OP, yytext); }
"^"                { addToken(XOR_OP, yytext); }
"~"                { addToken(BITWISE_NOT, yytext); }
"!"                { addToken(LOGICAL_NOT, yytext); }
"-"                { addToken(UNARY_MINUS, yytext); }
"&&"               { addToken(LOGICAL_AND, yytext); }
"||"               { addToken(LOGICAL_OR, yytext); }
";"                { addToken(SEMICOLON, yytext); }
","                { addToken(COMMA, yytext); }
"<"                { addToken(LESS_THAN, yytext); }
"<="               { addToken(LESS_THAN_EQUAL, yytext); }
">"                { addToken(GREATER_THAN, yytext); }
">="               { addToken(GREATER_THAN_EQUAL, yytext); }
"=="               { addToken(EQUAL, yytext); }
"!="               { addToken(NOT_EQUAL, yytext); }
"if"               {addToken(IF_KEYWORD,yytext);}
"else"             {addToken(ELSE_KEYWORD,yytext);}
"while"            {addToken(WHILE_KEYWORD,yytext);}
"continue"         {addToken(CONTINUE_KEYWORD,yytext);}
"break"            {addToken(BREAK_KEYWORD,yytext);}

[0-9]+             { addToken(INTEGER_LITERAL, yytext); }

[a-zA-Z_][a-zA-Z0-9_]* { addToken(IDENTIFIER, yytext); }

[ \t\n\r]+         { /* 忽略空格 */ }

.                  { addToken(UNKNOWN, yytext); }

%%

vector<Token> tokenize(const string &src) {
    tokens.clear();
    YY_BUFFER_STATE buffer = yy_scan_string(src.c_str());
    yy_switch_to_buffer(buffer);
    yylex();
    yy_delete_buffer(buffer);
    return tokens;
}