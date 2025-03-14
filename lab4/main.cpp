#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stack>
#include <string>
#include <vector>

#include "token.h"
#include "generate.h"
#include "postfix.h"

using namespace std;





int main(int argc, char *argv[])
{
    const vector<string> tokenstemp;
    if (argc != 2)
    {
        cout << "请提供文件地址作为参数" << endl;
        return 1;
    }
    string filePath = argv[1];
    ifstream inputFile(filePath);
    if (!inputFile)
    {
        cout << "无法打开文件" << endl;
        return 1;
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();
    string src = buffer.str();
    inputFile.close();

    vector<Token> tokens = tokenizer(src); // 词法分析

    generate(tokens); // 生成X86代码

    return 0;
}
