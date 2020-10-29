#include "Error.h"

Error::Error(ErrorType type, Token* token)
{
	this->token = token;
	this->type = type;
	this->line = token->getLine();
}

static map<ErrorType, char> ErrorMap = {
		{ a, 'a' },
		{ b, 'b' },
		{ c, 'c' },
		{ d, 'd' },
		{ e, 'e' },
		{ f, 'f' },
		{ g, 'g' },
		{ h, 'h' },
		{ i, 'i' },
		{ j, 'j' },
		{ k, 'k' },
		{ l, 'l' },
		{ m, 'm' },
		{ n, 'n' },
		{ o, 'o' },
		{ p, 'p' }
};

ostream& operator<<(ostream& err, const Error& e)
{
	err << ErrorMap[e.type] << ' ' << e.line << endl;
	return err;
}

Error::Error(ErrorType type, int line)
{
	this->token = nullptr;
	this->type = type;
	this->line = line;
}

vector <Error> errList;