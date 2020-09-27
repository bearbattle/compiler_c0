#include "lexer.h"

ifstream inputFile("testfile.txt");
ofstream outputFile("output.txt");

int main() {
	while (!inputFile.eof()) {
		getChar();
		outputFile << getToken() << endl;
	}
	inputFile.close();
	outputFile.close();
	return 0;
}