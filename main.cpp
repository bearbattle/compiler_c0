#include "lexer.h"
#include "parser.h"
#include "SymbolTable.h"

ifstream inputFile;
ofstream outputFile;

int main() {
    initSymTabs();
    inputFile.open("testfile.txt");
    outputFile.open("output.txt");
    /*if (!inputFile.eof()) {
        getChar();
    }*/
    lexer::getToken();
    program();
    inputFile.close();
    outputFile.close();
    return 0;
}