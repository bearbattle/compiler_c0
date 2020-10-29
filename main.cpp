#include "lexer.h"
#include "parser.h"
#include "SymbolTable.h"
#include "Error.h"

ifstream inputFile;
ofstream outputFile;
ofstream errorFile;

int main()
{
    initSymTabs();
    inputFile.open("testfile.txt");
    outputFile.open("output.txt");
    errorFile.open("error.txt");
    /* if (!inputFile.eof()) {
        getChar();
    }*/
    lexer::getToken();
    program();
    for (Error e : errList)
    {
        errorFile << e;
    }
    inputFile.close();
    outputFile.close();
    return 0;
}