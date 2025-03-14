#include "token.h"

extern vector<Token> tokenize(const string &src);


vector<Token> tokenizer(const string &src){
    vector<Token> tks = tokenize(src);
    // for(int i=0; i<tks.size(); i++){
    //    cout<<"No."<<i<<" type: "<<tks[i].type<<" value: "<<tks[i].value<<"\n";
    // }
    return tks;
}