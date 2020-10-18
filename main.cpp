#include "lexer.h"
#include "parser.h"

ifstream inputFile;
ofstream outputFile;

int main() {
    inputFile.open("testfile.txt");
    outputFile.open("output.txt");
    lexer::getToken();
    program();
    inputFile.close();
    outputFile.close();
    return 0;
}