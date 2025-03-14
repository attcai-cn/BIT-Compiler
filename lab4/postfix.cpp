#include "token.h"
#include <stack>
#include <vector>

// 判断是否为运算符
bool isOperator(TokenType type)
{
    bool flag = (type == TokenType::LESS_THAN || type == TokenType::LESS_THAN_EQUAL ||
                 type == TokenType::GREATER_THAN || type == TokenType::GREATER_THAN_EQUAL || type == TokenType::EQUAL ||
                 type == TokenType::NOT_EQUAL || type == TokenType::PLUS_OP || type == TokenType::MINUS_OP ||
                 type == TokenType::MULTIPLY_OP || type == TokenType::DIVIDE_OP || type == TokenType::MODULO_OP ||
                 type == TokenType::AND_OP || type == TokenType::OR_OP || type == TokenType::XOR_OP ||
                 type == TokenType::LOGICAL_AND || type == TokenType::LOGICAL_OR || type == TokenType::BITWISE_NOT ||
                 type == TokenType::UNARY_MINUS || type == TokenType::LOGICAL_NOT);
    return flag;
}
// 运算符优先级
int getPrecedence(TokenType type)
{
    switch (type)
    {
    case TokenType::UNARY_MINUS:
    case TokenType::LOGICAL_NOT:
    case TokenType::BITWISE_NOT:
    case TokenType::FUNC_CALL:
        return 8;
    case TokenType::MULTIPLY_OP:
    case TokenType::DIVIDE_OP:
    case TokenType::MODULO_OP:
        return 7;
    case TokenType::PLUS_OP:
    case TokenType::MINUS_OP:
        return 6;
    case TokenType::LESS_THAN:
    case TokenType::LESS_THAN_EQUAL:
    case TokenType::GREATER_THAN:
    case TokenType::GREATER_THAN_EQUAL:
        return 5;
    case TokenType::EQUAL:
    case TokenType::NOT_EQUAL:
        return 4;
    case TokenType::AND_OP:
        return 3;
    case TokenType::XOR_OP:
    case TokenType::LOGICAL_AND:
        return 2;
    case TokenType::OR_OP:
    case TokenType::LOGICAL_OR:
        return 1;
    default:
        return 0;
    }
}

// 中缀表达式转后缀表达式
vector<Token> infixToPostfix(const vector<Token> &infix1)
{
    stack<Token> opStack;
    vector<Token> postfix;
    vector<Token> infix = infix1;

    cout << "#  INFIX" << infix.size() << ": ";
    for (auto token : infix)
    {
        cout << token.value << " ";
    }
    cout << endl;

    if (infix.size() == 1 && (infix[0].type == TokenType::INTEGER_LITERAL || infix[0].type == TokenType::IDENTIFIER))
    {
        postfix.push_back(infix[0]);
        return postfix;
    }
    Token endToken;
    endToken.type = TokenType::UNKNOWN;
    endToken.value = "END";

    infix.push_back(endToken);
    for (int i = 0; i < infix.size(); i++)
    {
        Token token = infix[i];

        // 函数调用,将函数调用整体作为一个token
        if (token.type == TokenType::IDENTIFIER && infix[i + 1].type == TokenType::LEFT_PAREN)
        {
            int j = i;
            string funcCall = "";
            Token funcToken;

            while (infix[j].type != TokenType::RIGHT_PAREN)
            {
                funcCall += infix[j].value;
                j++;
            }
            funcCall += ")";
            funcToken.value = funcCall;
            funcToken.type = TokenType::FUNC_CALL;
            postfix.push_back(funcToken);
            i = j;
        }
        else if (token.type == TokenType::IDENTIFIER || token.type == TokenType::INTEGER_LITERAL)
        {
            postfix.push_back(token);
        }
        else if (token.type == TokenType::LEFT_PAREN)
        {
            opStack.push(token);
        }
        else if (token.type == TokenType::RIGHT_PAREN)
        {
            while (!opStack.empty() && opStack.top().type != TokenType::LEFT_PAREN)
            {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty())
                opStack.pop(); // 弹出左括号
        }
        else if (isOperator(token.type))
        {
            while (!opStack.empty() && getPrecedence(opStack.top().type) >= getPrecedence(token.type))
            {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(token);
        }
    }

    while (!opStack.empty())
    {
        postfix.push_back(opStack.top());
        opStack.pop();
    }
    return postfix;
}
