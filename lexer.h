#ifndef _COMPILER_C0_LEXER
#define _COMPILER_C0_LEXER

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>
#include <deque>
#include "Token.h"

using namespace std;

// IO File Streams
extern ifstream inputFile;
extern ofstream outputFile;

namespace lexer {
    extern Token *prevToken;
    extern Token *curToken;
    extern deque<Token *> afterWards;

    Token *preFetch(int amount);

    Token *_getToken();

    void getToken();

    TokenType getTokenType();
}

void getChar();

#endif // !_COMPILER_C0_LEXER
