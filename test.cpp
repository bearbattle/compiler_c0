#include <iostream>
#include <fstream>

using namespace std;

ifstream inputFile;
ofstream outputFile;

int main() {
    inputFile.open("testfile.txt");
    outputFile.open("output.txt");
    while (!inputFile.eof()) {
        outputFile << ((int)(inputFile.get())) << endl;
    }
    inputFile.close();
    outputFile.close();
    return 0;
}