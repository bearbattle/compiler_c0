#ifndef COMPILER_0_ERROR_H
#define COMPILER_0_ERROR_H

#include <exception>
#include <fstream>
#include <vector>
#include "Token.h"

using namespace std;

extern ofstream errorFile;

enum ErrorType
{
	a, b, c, d, e, f, g,
	h, i, j, k, l, m, n, o, p
};

class Error
{
private:
	ErrorType type;
	Token* token;
	int line;
public:
	Error(ErrorType type, Token* token);

	Error(ErrorType type, int line);

	friend ostream& operator<<(ostream& err, const Error& e);
};

extern vector <Error> errList;

#endif //COMPILER_0_ERROR_H
