#include "lexer.h"
#include "parser.h"
#include "SymbolTable.h"
#include "Error.h"
#include "MidCode.h"
#include "ToMips.h"

ifstream inputFile;
ofstream outputFile;
ofstream errorFile;
ofstream midCodeFileInit;
ofstream mipsFile;

int main()
{
    initSymTabs();
    inputFile.open("testfile.txt");
    outputFile.open("output.txt");
    errorFile.open("error.txt");
    midCodeFileInit.open("midCode.txt");
    mipsFile.open("mips.txt");
    /* if (!inputFile.eof()) {
        getChar();
    }*/
    lexer::getToken();
    program();
    for (Error e : errList)
    {
        errorFile << e;
    }
    for (auto midCode : midCodes)
    {
        midCode->out(midCodeFileInit);
        midCodeFileInit << endl;
    }
    toMips();
    inputFile.close();
    outputFile.close();
    errorFile.close();
    midCodeFileInit.close();
    mipsFile.close();
    return 0;
}