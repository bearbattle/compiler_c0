#include "lexer.h"

ifstream inputFile;
ofstream outputFile;

int main() {
    inputFile.open("testfile.txt");
    outputFile.open("output.txt");
    while (!inputFile.eof()) {
        getChar();
        Token* ptk = getToken();
        if (ptk != nullptr) {
            outputFile << *ptk << endl;
        }
    }
    inputFile.close();
    outputFile.close();
    return 0;
}