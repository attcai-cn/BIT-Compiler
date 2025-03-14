#include "generate.h"
#include "postfix.h"
#include "token.h"
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>

using namespace std;

int lflag = 0;

// 在局部变量和参数中查找变量的偏移值，拼接上正或负号，形成字符串返回
string getPosByIdentifier(string ident, const map<string, int> &varMap, const map<string, int> &funcVarMap)
{
    int temp;
    string ret = "";
    if (varMap.find(ident) == varMap.end())
    {
        temp = funcVarMap.at(ident);
        ret.append("+");
    }
    else
    {
        temp = varMap.at(ident);
        ret.append("-");
    }
    ret.append(to_string(temp));
    return ret;
}

// 根据函数调用生成代码
void generateFromFunction(const vector<Token> &tokens, const map<string, int> &varMap,
                          const map<string, int> &funcArgMap, int &i, const string &varName)
{
    string funcName = tokens[i].value;
    int argCnt = 0;
    i += 2;
    // int isInFunc = 0;
    cout << "#(func) " + varName + " = " + funcName + "()\n";

    int layer = 0;
    while (tokens[i].type != TokenType::RIGHT_PAREN || layer != 0)
    {
        if (tokens[i].type == TokenType::LEFT_PAREN)
        {
            layer++;
        }
        else if (tokens[i].type == TokenType::RIGHT_PAREN)
        {
            layer--;
        }
        if (layer == 0 && tokens[i].type == TokenType::COMMA)
        {
            argCnt++;
        }
        i++;
    }
    argCnt++;
    i -= 1;

    layer = 0;
    while (tokens[i].type != TokenType::LEFT_PAREN || layer != 0)
    {

        while (layer != 0 || (tokens[i].type != TokenType::COMMA && tokens[i].type != TokenType::LEFT_PAREN))
        {
            if (tokens[i].type == TokenType::RIGHT_PAREN)
            {
                layer++;
            }
            else if (tokens[i].type == TokenType::LEFT_PAREN)
            {
                layer--;
            }
            i--;
        }
        // cout << "#   after shift i: " << i << endl;
        layer = 0;
        vector<Token> temp;
        int j = i + 1;
        while ((tokens[j].type != TokenType::COMMA && tokens[j].type != TokenType::RIGHT_PAREN) || layer != 0)
        {
            if (tokens[j].type == TokenType::LEFT_PAREN)
            {
                layer++;
            }
            else if (tokens[j].type == TokenType::RIGHT_PAREN)
            {
                layer--;
            }
            int layer1 = 0;
            // 嵌套函数
            if (tokens[j].type == TokenType::IDENTIFIER && tokens[j + 1].type == TokenType::LEFT_PAREN)
            {
                while (tokens[j].type != TokenType::RIGHT_PAREN || layer1 != 0)
                {
                    if (tokens[j].type == TokenType::LEFT_PAREN)
                    {
                        layer1++;
                    }
                    else if (tokens[j].type == TokenType::RIGHT_PAREN)
                    {
                        layer1--;
                    }
                    temp.push_back(tokens[j]);
                    j++;
                }
                temp.push_back(tokens[j]);
                j++;
                continue;
            }
            temp.push_back(tokens[j]);
            j++;
        }

        vector<Token> postfix = infixToPostfix(temp);
        temp.clear();
        // cout << "#  call in func\n";
        generateFromPostfix(postfix, varMap, funcArgMap, "", false, true);
        if (tokens[i].type == TokenType::COMMA)
        {
            i -= 1;
        }
    }
    while (tokens[i].type != TokenType::RIGHT_PAREN)
    {
        i++;
    }

    cout << "    call " << funcName << endl;
    cout << "    add esp, " << 4 * argCnt << endl;
    if (varName != "")
    {
        cout << "    mov DWORD PTR [ebp" << getPosByIdentifier(varName, varMap, funcArgMap) << "], eax" << endl;
    }
}

// 根据后缀表达式生成代码
void generateFromPostfix(const vector<Token> &postfix, const map<string, int> &varMap, const map<string, int> &argMap,
                         const string &resultVar, bool isReturn, bool isFuncCall)
{
    int regCount = 0;
    int isInt = 0;
    string reg[2] = {"eax", "ebx"};
    string resultOffset = "";
    if (resultVar != "")
    {
        resultOffset = getPosByIdentifier(resultVar, varMap, argMap);
    }

    for (const Token &token : postfix)
    {

        if (token.type == TokenType::INTEGER_LITERAL && postfix.size() == 1 && !isFuncCall)
        {
            isInt = 1;
            cout << "    mov " << reg[0] << ", " << token.value << endl;
            cout << "    mov DWORD PTR [ebp" << resultOffset << "], " << reg[0] << endl;
            break;
        }
        if (postfix.size() == 1 && isFuncCall && token.type == TokenType::INTEGER_LITERAL)
        {
            // cout << "    mov " << reg[0] << ", " << token.value << endl;
            // cout << "    push " << reg[0] << endl;
            cout << "    push " << token.value << endl;
            break;
        }

        if (token.type == TokenType::IDENTIFIER)
        {
            cout << "    mov eax" << ", DWORD PTR [ebp" << getPosByIdentifier(token.value, varMap, argMap) << "]"
                 << "  # 取出" + token.value << endl;
            cout << "    push eax" << endl;
            // regCount = (regCount + 1) % 2;
        }
        else if (token.type == TokenType::INTEGER_LITERAL)
        {
            cout << "    mov eax, " << token.value << endl;
            cout << "    push eax" << endl;
            // regCount = (regCount + 1) % 2;
        }
        else if (token.type == TokenType::FUNC_CALL)
        {
            // cout<<"TOKEN: " <<token.value  << endl;
            int i = 0;
            vector<Token> temp;
            temp = tokenizer(token.value);
            // for (int i = 0; i < temp.size(); i++)
            // {
            //     cout << "# " << temp[i].value << endl;
            // }
            generateFromFunction(temp, varMap, argMap, i, "");
            cout << "    push eax" << endl;
            if (postfix.size() == 1)
            {
                break;
            }
        }
        else if (token.type == TokenType::UNARY_MINUS || token.type == TokenType::LOGICAL_NOT ||
                 token.type == TokenType::BITWISE_NOT)
        {

            switch (token.type)
            {
            case TokenType::UNARY_MINUS: {
                cout << "    pop eax" << endl;
                cout << "    neg eax" << endl;
                cout << "    push eax" << endl;
            }
            break;
            case TokenType::LOGICAL_NOT: {
                cout << "    pop eax" << endl;
                cout << "    cmp eax, 0" << endl;
                cout << "    sete al" << endl;
                cout << "    movzx eax, al" << endl;
                cout << "    push eax" << endl;
            }
            break;
            case TokenType::BITWISE_NOT: {
                cout << "    pop eax" << endl;
                cout << "    not eax" << endl;
                cout << "    push eax" << endl;
            }
            break;
            }
        }
        else if (isOperator(token.type))
        {
            string operand1 = "eax";
            string operand2 = "ebx";
            string resultReg = "eax";

            if (token.value == "+")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    add " << operand1 << ", " << operand2 << "  # 加法" << endl;
                cout << "    push " << operand1 << endl;
                // cout << "    add " << resultReg << ", " << operand1 << ", " <<
                // operand2 << endl;
            }
            else if (token.value == "-" && regCount == 0)
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    sub " << operand1 << ", " << operand2 << "  # 减法" << endl;
                cout << "    push " << operand1 << endl;
                // cout << "    sub " << resultReg << ", " << operand1 << ", " <<
                // operand2 << endl;
            }
            else if (token.value == "*")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    imul " << operand1 << ", " << operand2 << "  # 乘法" << endl;
                cout << "    push " << operand1 << endl;
                // cout << "    imul " << resultReg << ", " << operand1 << ", " <<
                // operand2 << endl;
            }
            else if (token.value == "/")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    cdq" << endl;
                cout << "    idiv " << operand2 << "  # 除法" << endl;
                cout << "    push eax" << endl;
                // cout << "    mov eax, " << operand1 << endl;
            }
            else if (token.value == "%")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    cdq" << endl;
                cout << "    idiv " << operand2 << "  # 取余" << endl;
                cout << "    push edx" << endl;
                // cout << "    mov eax, " << operand1 << endl;
            }
            else if (token.value == "<")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    cmp " << operand1 << ", " << operand2 << endl;
                cout << "    setl al" << endl;
                cout << "    movzx " << resultReg << ", al" << endl;
                cout << "    push " << resultReg << endl;
            }
            else if (token.value == "<=")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    cmp " << operand1 << ", " << operand2 << endl;
                cout << "    setle al" << endl;
                cout << "    movzx " << resultReg << ", al" << endl;
                cout << "    push " << resultReg << endl;
            }
            else if (token.value == ">")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    cmp " << operand1 << ", " << operand2 << endl;
                cout << "    setg al" << endl;
                cout << "    movzx " << resultReg << ", al" << endl;
                cout << "    push " << resultReg << endl;
            }
            else if (token.value == ">=")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    cmp " << operand1 << ", " << operand2 << endl;
                cout << "    setge al" << endl;
                cout << "    movzx " << resultReg << ", al" << endl;
                cout << "    push " << resultReg << endl;
            }
            else if (token.value == "==")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    cmp " << operand1 << ", " << operand2 << endl;
                cout << "    sete al" << endl;
                cout << "    movzx " << resultReg << ", al" << endl;
                cout << "    push " << resultReg << endl;
            }
            else if (token.value == "!=")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    cmp " << operand1 << ", " << operand2 << endl;
                cout << "    setne al" << endl;
                cout << "    movzx " << resultReg << ", al" << endl;
                cout << "    push " << resultReg << endl;
            }
            else if (token.value == "&")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    and " << operand1 << ", " << operand2 << "  # 与" << endl;
                cout << "    push " << operand1 << endl;
            }
            else if (token.value == "|")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    or " << operand1 << ", " << operand2 << "  # 逻辑或" << endl;
                cout << "    push " << operand1 << endl;
                // cout << "    or " << resultReg << ", " << operand1 << ", " <<
                // operand2 << endl;
            }
            else if (token.value == "^")
            {
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;
                cout << "    xor " << operand1 << ", " << operand2 << "  # 异或" << endl;
                cout << "    push " << operand1 << endl;
                // cout << "    xor " << resultReg << ", " << operand1 << ", " <<
                // operand2 << endl;
            }
            else if (token.value == "&&")
            {
                string l1 = ".L" + to_string(lflag++);
                string l2 = ".L" + to_string(lflag++);
                string l3 = ".L" + to_string(lflag++);
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;

                cout << "    cmp ebx, 0" << endl;
                cout << "    je " << l1 << endl;
                cout << "    cmp eax, 0" << endl;
                cout << "    je " << l1 << endl;
                cout << "    mov eax, 1" << endl;
                cout << "    jmp " << l3 << endl;
                cout << "  " << l1 << ":" << endl;
                cout << "    mov eax, 0" << endl;
                cout << "  " << l3 << ":" << endl;
                cout << "    push eax" << endl;
            }
            else if (token.value == "||")
            {
                string l1 = ".L" + to_string(lflag++);
                string l2 = ".L" + to_string(lflag++);
                string l3 = ".L" + to_string(lflag++);
                // string l4 = ".L" + to_string(lflag++);
                cout << "    pop " << operand2 << endl;
                cout << "    pop " << operand1 << endl;

                cout << "    cmp ebx, 0" << endl;
                cout << "    je " << l1 << endl;
                cout << "    mov eax, 1" << endl;
                cout << "    jmp " << l3 << endl;
                cout << "  " << l1 << ":" << endl;
                cout << "    cmp eax, 0" << endl;
                cout << "    je " << l2 << endl;
                cout << "    mov eax, 1" << endl;
                cout << "    jmp " << l3 << endl;
                cout << "  " << l2 << ":" << endl;
                cout << "    mov eax, 0" << endl;
                cout << "  " << l3 << ":" << endl;
                cout << "    push eax" << endl;
            }
        }
    }

    if (isInt || isFuncCall)
    {
        return;
    }
    cout << "    pop eax" << endl;
    if (isReturn)
    {
        return;
    }
    cout << "    mov DWORD PTR [ebp" << resultOffset << "], eax" << endl;
}

// 根据函数调用生成代码

// 根据Token序列生成代码
void generate(const vector<Token> &tokens)
{
    map<string, int> varMap;     // 局部变量
    map<string, int> funcMap;    // 函数
    map<string, int> funcArgMap; // 函数参数
    int variableOffset = 0;
    int funcVarOffset = 4;
    vector<Token> expression;
    int isInFunc = 0;    // 是否在函数中
    int isVoid = 0;      // 是否为void函数
    int isIf = 0;        // 是否为if语句
    stack<int> ifFlag;   // if语句跳转标志
    stack<int> whileFlag; // while语句跳转标志
    int isElse = 0;      // 是否为else语句
    int isWhile = 0;     // 是否为while语句
    int endVar = 114514; // if、while语句break跳转标志
    stack<int> ctrlFlag; // 记录if、while语句的嵌套顺序，0=if,1=while

    // 预扫描，找到函数
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type == TokenType::INT_KEYWORD && tokens[i + 1].type == TokenType::IDENTIFIER &&
            tokens[i + 2].type == TokenType::LEFT_PAREN)
        {
            funcMap[tokens[i + 1].value] = 1;
        }
        else if (tokens[i].type == TokenType::VOID_KEYWORD && tokens[i + 1].type == TokenType::IDENTIFIER &&
                 tokens[i + 2].type == TokenType::LEFT_PAREN)
        {
            funcMap[tokens[i + 1].value] = 1;
        }
    }

    // 初始化
    cout << ".intel_syntax noprefix" << endl;
    cout << ".global main" << endl;
    for (int i = funcMap.size(); i > 0; i--)
    {
        cout << ".extern " << funcMap.begin()->first << endl;
        funcMap.erase(funcMap.begin());
    }
    cout << ".extern printf" << endl;
    cout << ".data" << endl;
    cout << "format_str:\n  .asciz \"\%d\\n\"" << endl;
    cout << ".text" << endl;

    for (int i = 0; i < tokens.size(); ++i)
    {
        // cout << "#   i:" << i << " isInFunc:" << isInFunc << "  isVOid:" << isVoid << endl;
        // 遇到一个新的函数
        if (isInFunc == 0)
        {
            // 处理main函数
            if (tokens[i].type == TokenType::INT_KEYWORD && tokens[i + 1].type == TokenType::MAIN_KEYWORD &&
                tokens[i + 2].type == TokenType::LEFT_PAREN && tokens[i + 3].type == TokenType::RIGHT_PAREN)
            {
                cout << "main:" << endl;
                cout << "    push ebp" << endl;
                cout << "    mov ebp, esp" << endl;
                cout << "    sub esp, 0x100" << endl;
                i += 4;
                isInFunc = 1;
                continue;
            }
            // 处理main函数(带参数)
            else if (tokens[i].type == TokenType::INT_KEYWORD && tokens[i + 1].type == TokenType::MAIN_KEYWORD &&
                     tokens[i + 2].type == TokenType::LEFT_PAREN && tokens[i + 3].type == TokenType::INT_KEYWORD)
            {
                cout << "main:" << endl;
                cout << "    push ebp" << endl;
                cout << "    mov ebp, esp" << endl;
                cout << "    sub esp, 0x100" << endl;
                i += 8;
                isInFunc = 1;
                continue;
            }
            // 处理函数声明
            else if ((tokens[i].type == TokenType::INT_KEYWORD || tokens[i].type == TokenType::VOID_KEYWORD) &&
                     tokens[i + 1].type == TokenType::IDENTIFIER && tokens[i + 2].type == TokenType::LEFT_PAREN)
            {
                if (tokens[i].type == TokenType::VOID_KEYWORD)
                {
                    isVoid = 1;
                }
                // 不带参数
                if (tokens[i + 3].type == TokenType::RIGHT_PAREN)
                {
                    cout << tokens[i + 1].value << ":" << endl;
                    cout << "    push ebp" << endl;
                    cout << "    mov ebp, esp" << endl;
                    cout << "    sub esp, 4" << endl;
                    i += 4;
                    isInFunc = 1;
                    continue;
                }
                else // 带参数
                {
                    cout << tokens[i + 1].value << ":" << endl;
                    cout << "    push ebp" << endl;
                    cout << "    mov ebp, esp" << endl;
                    cout << "    sub esp, 4" << endl;
                    isInFunc = 1;
                    i += 3;
                    while (tokens[i].type != TokenType::RIGHT_PAREN)
                    {
                        if (tokens[i].type == TokenType::INT_KEYWORD)
                        {
                            funcVarOffset += 4;
                            funcArgMap[tokens[i + 1].value] = funcVarOffset;
                        }
                        i++;
                    }
                    i += 1;
                }
            }
        }
        else
        {
            // 函数的结束
            if (tokens[i].type == TokenType::RIGHT_BRACE && i + 1 >= tokens.size())
            {
                string l1 = ".L" + to_string(endVar);
                cout << l1 << ":" << endl;
                cout << "    leave" << endl;
                cout << "    ret\n\n" << endl;
                isInFunc = 0;
                funcVarOffset = 4;
                variableOffset = 0;
                isVoid = 0;
                varMap.clear();
                funcArgMap.clear();
                endVar--;
                continue;
            }
            else if (tokens[i].type == TokenType::RIGHT_BRACE &&
                     (tokens[i + 1].type == TokenType::INT_KEYWORD || tokens[i + 1].type == TokenType::VOID_KEYWORD) &&
                     (tokens[i + 2].type == TokenType::IDENTIFIER || tokens[i + 2].type == TokenType::MAIN_KEYWORD))
            {
                string l1 = ".L" + to_string(endVar);
                cout << l1 << ":" << endl;
                cout << "    leave" << endl;
                cout << "    ret\n\n" << endl;
                isInFunc = 0;
                funcVarOffset = 4;
                variableOffset = 0;
                isVoid = 0;
                varMap.clear();
                funcArgMap.clear();
                endVar--;
                continue;
            }
            // 处理if语句
            if (tokens[i].type == TokenType::IF_KEYWORD)
            {
                cout << "# if @" << i << endl;
                ctrlFlag.push(0);
                ifFlag.push(lflag);
                string l1 = ".L" + to_string(lflag++);
                isIf += 1;
                vector<Token> temp;
                i += 2;
                int layer = 0;
                while (tokens[i].type != TokenType::RIGHT_PAREN || layer != 0)
                {
                    if (tokens[i].type == TokenType::LEFT_PAREN)
                    {
                        layer++;
                    }
                    else if (tokens[i].type == TokenType::RIGHT_PAREN)
                    {
                        layer--;
                    }
                    temp.push_back(tokens[i]);
                    i++;
                }
                vector<Token> postfix = infixToPostfix(temp);
                generateFromPostfix(postfix, varMap, funcArgMap, "", true, false);
                cout << "    cmp eax, 1" << endl;
                cout << "    jl " << l1 << endl;
            }
            // if语句结束
            if (tokens[i].type == TokenType::RIGHT_BRACE && isIf == 1 && ctrlFlag.top() == 0)
            {
                if (tokens[i + 1].type != TokenType::ELSE_KEYWORD)
                {
                    ctrlFlag.pop();
                    string l1 = ".L" + to_string(ifFlag.top());
                    ifFlag.pop();
                    cout << l1 << ":" << endl;
                    isIf -= 1;
                    cout << "# if ends" << endl;
                }
                else
                {
                    string l1 = ".L" + to_string(ifFlag.top());
                    ifFlag.pop();
                    isIf -= 1;
                    ifFlag.push(lflag);
                    string l2 = ".L" + to_string(lflag++);
                    cout << "    jmp " << l2 << endl;
                    cout << l1 << ":" << endl;
                }

                continue;
            }
            // 处理else语句
            if (tokens[i].type == TokenType::ELSE_KEYWORD)
            {
                cout << "# else" << endl;
                isElse += 1;
                i += 1;
            }
            // else语句结束
            if (tokens[i].type == TokenType::RIGHT_BRACE && isElse && ctrlFlag.top() == 0)
            {
                ctrlFlag.pop();
                string l1 = ".L" + to_string(ifFlag.top());
                ifFlag.pop();
                cout << l1 << ":" << endl;
                isElse -= 1;
                cout << "# if ends" << endl;

                continue;
            }
            // 处理while语句
            if (tokens[i].type == TokenType::WHILE_KEYWORD)
            {
                ctrlFlag.push(1);
                cout << "# while" << endl;
                whileFlag.push(lflag);
                string l1 = ".L" + to_string(lflag++); // while开始
                whileFlag.push(lflag);
                string l2 = ".L" + to_string(lflag++); // while结束
                cout << l1 << ":" << endl;

                isWhile += 1;
                vector<Token> temp;
                i += 2;
                int layer = 0;
                while (tokens[i].type != TokenType::RIGHT_PAREN || layer != 0)
                {
                    if (tokens[i].type == TokenType::LEFT_PAREN)
                    {
                        layer++;
                    }
                    else if (tokens[i].type == TokenType::RIGHT_PAREN)
                    {
                        layer--;
                    }
                    temp.push_back(tokens[i]);
                    i++;
                }
                vector<Token> postfix = infixToPostfix(temp);
                generateFromPostfix(postfix, varMap, funcArgMap, "", true, false);
                cout << "    cmp eax, 1" << endl;
                cout << "    jl " << l2 << endl;
            }
            // while语句结束
            if (tokens[i].type == TokenType::RIGHT_BRACE && isWhile && ctrlFlag.top() == 1)
            {
                ctrlFlag.pop();
                cout << "# while ends" << endl;
                string l1 = ".L" + to_string(whileFlag.top());
                whileFlag.pop();
                string l2 = ".L" + to_string(whileFlag.top());
                whileFlag.pop();
                cout << "    jmp " << l2 << endl;
                cout << l1 << ":" << endl;
                isWhile -= 1;
                continue;
            }
            // 处理continue语句
            if (tokens[i].type == TokenType::CONTINUE_KEYWORD)
            { // 临时出栈，得到while开始的标志，再入栈
                int tp = whileFlag.top();
                whileFlag.pop();
                string l1 = ".L" + to_string(whileFlag.top());
                cout << "    jmp " << l1 <<" # continue"<< endl;
                whileFlag.push(tp);
            }
            // 处理break语句
            if (tokens[i].type == TokenType::BREAK_KEYWORD)
            {
                string l1 = ".L" + to_string(whileFlag.top());
                cout << "    jmp " << l1 <<" # break" <<endl;
            }
            // 处理变量声明
            // 处理变量声明+赋值:int a=1,b=1+2,c=a; ...
            if (tokens[i].type == TokenType::INT_KEYWORD && tokens[i + 1].type == TokenType::IDENTIFIER &&
                (tokens[i + 2].type == TokenType::ASSIGN_OP || tokens[i + 2].type == TokenType::SEMICOLON ||
                 tokens[i + 2].type == TokenType::COMMA))
            {
                i += 1;
                while (tokens[i].type != TokenType::SEMICOLON)
                {
                    string varName = tokens[i].value;
                    variableOffset += 4;
                    varMap[varName] = variableOffset;
                    if (tokens[i + 1].type == TokenType::COMMA)
                    {
                        i += 2;
                        continue;
                    }
                    else if (tokens[i + 1].type == TokenType::SEMICOLON)
                    {
                        break;
                    }
                    i += 2;

                    while (tokens[i].type != TokenType::COMMA && tokens[i].type != TokenType::SEMICOLON)
                    {
                        // 因为函数里面也有逗号，所以要判断是否是函数调用
                        if (tokens[i].type == TokenType::IDENTIFIER && tokens[i + 1].type == TokenType::LEFT_PAREN)
                        {
                            while (tokens[i].type != TokenType::RIGHT_PAREN)
                            {
                                expression.push_back(tokens[i]);
                                i++;
                            }
                            expression.push_back(tokens[i]);
                            i++;
                            continue;
                        }
                        expression.push_back(tokens[i]);
                        i++;
                    }
                    // for(auto exp : expression){
                    //     cout<<exp.value<<" ";
                    // }
                    cout << endl;
                    vector<Token> postfix = infixToPostfix(expression);
                    expression.clear(); // 清空表达式
                    generateFromPostfix(postfix, varMap, funcArgMap, varName);

                    if (tokens[i].type == TokenType::COMMA)
                    {
                        i += 1; // 逗号继续向前搜索
                    }
                }
            }
            // 处理函数直接调用
            else if (tokens[i].type == TokenType::IDENTIFIER && tokens[i + 1].type == TokenType::LEFT_PAREN)
            {
                // func()
                if (tokens[i + 2].type == TokenType::RIGHT_PAREN)
                {
                    cout << "    call " << tokens[i].value << endl;
                    i += 3;
                }
                // func(a,b,c)
                else
                {
                    generateFromFunction(tokens, varMap, funcArgMap, i, "");
                }
                continue;
            }
            // 处理赋值语句
            else if (tokens[i].type == TokenType::IDENTIFIER && tokens[i + 1].type == TokenType::ASSIGN_OP)
            {
                string varName = tokens[i].value;
                i += 2;
                while (i < tokens.size() && tokens[i].type != TokenType::SEMICOLON)
                {
                    expression.push_back(tokens[i]);
                    ++i;
                }
                vector<Token> postfix = infixToPostfix(expression);
                expression.clear(); // 清空表达式
                generateFromPostfix(postfix, varMap, funcArgMap, varName);
            }
            // 处理printf函数
            else if (tokens[i].type == TokenType::PRINTLN_INT)
            {
                cout << "# call printf" << endl;

                vector<Token> temp;
                i += 2;
                int layer = 0;
                while (tokens[i].type != TokenType::RIGHT_PAREN || layer != 0)
                {
                    if (tokens[i].type == TokenType::LEFT_PAREN)
                    {
                        layer++;
                    }
                    else if (tokens[i].type == TokenType::RIGHT_PAREN)
                    {
                        layer--;
                    }
                    temp.push_back(tokens[i]);
                    i++;
                }
                vector<Token> postfix = infixToPostfix(temp);
                generateFromPostfix(postfix, varMap, funcArgMap, "", false, true);

                cout << "    push offset format_str" << endl;
                cout << "    call printf" << endl;
                cout << "    add esp, 8" << endl;
                // i += 4;
                // cout << "# i:" << i << endl;
                continue;
            }
            // 处理return语句
            else if (tokens[i].type == TokenType::RETURN_KEYWORD)
            {
                // return;
                if (isVoid)
                {
                    if (isIf || isWhile)
                    {
                        string l1 = ".L" + to_string(endVar);
                        cout << "    jmp " << l1 << endl;
                    }
                    continue;
                }
                // return a; 或者 return 1;
                if (tokens[i + 2].type == TokenType::SEMICOLON)
                {
                    if (tokens[i + 1].type == TokenType::IDENTIFIER)
                    {
                        cout << "    mov eax, DWORD PTR [ebp"
                             << getPosByIdentifier(tokens[i + 1].value, varMap, funcArgMap) << "]" << endl;
                    }
                    else if (tokens[i + 1].type == TokenType::INTEGER_LITERAL)
                    {
                        cout << "    mov eax, " << tokens[i + 1].value << endl;
                    }
                    i += 2;
                }
                else
                {
                    // return a+b; return a+func();
                    while (tokens[i].type != TokenType::SEMICOLON)
                    {
                        expression.push_back(tokens[i]);
                        i++;
                    }
                    vector<Token> postfix = infixToPostfix(expression);
                    expression.clear(); // 清空表达式
                    generateFromPostfix(postfix, varMap, funcArgMap, "", true, false);
                }
                if (isIf || isWhile)
                {
                    string l1 = ".L" + to_string(endVar);
                    cout << "    jmp " << l1 << endl;
                }
            }
        }
    }
    cout << endl;
}
