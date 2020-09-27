#ifndef _COMPILER_C0_LEXER
#define _COMPILER_C0_LEXER

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>

using namespace std;

// Defines the types of token
enum TokenType
{
	IDENFR, INTCON, CHARCON, STRCON, CONSTTK,
	INTTK, CHARTK, VOIDTK, MAINTK,
	IFTK, ELSETK, SWITCHTK, CASETK, DEFAULTTK,
	WHILETK, FORTK,
	SCANFTK, PRINTFTK,
	RETURNTK,
	PLUS, MINU, MULT, DIV,
	LSS, LEQ, GRE, GEQ, EQL, NEQ,
	COLON, ASSIGN, SEMICN, COMMA,
	LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
};

// Map the token type to string
static map<TokenType, string> tokenStringMap = {
	{IDENFR,"IDENFR"}, {INTCON,"INTCON"}, {CHARCON,"CHARCON"}, {STRCON,"STRCON"}, {CONSTTK,"CONSTTK"},
	{INTTK,"INTTK"}, {CHARTK,"CHARTK"}, {VOIDTK,"VOIDTK"}, {MAINTK,"MAINTK"},
	{IFTK,"IFTK"}, {ELSETK,"ELSETK"}, {SWITCHTK,"SWITCHTK"}, {CASETK,"CASETK"}, {DEFAULTTK,"DEFAULTTK"},
	{WHILETK,"WHILETK"}, {FORTK,"FORTK"},
	{SCANFTK,"SCANFTK"}, {PRINTFTK,"PRINTFTK"},
	{RETURNTK,"RETURNTK"},
	{PLUS,"PLUS"}, {MINU,"MINU"}, {MULT,"MULT"}, {DIV,"DIV"},
	{LSS,"LSS"}, {LEQ,"LEQ"}, {GRE,"GRE"}, {GEQ,"GEQ"}, {EQL,"EQL"}, {NEQ,"NEQ"},
	{COLON,"COLON"}, {ASSIGN,"ASSIGN"}, {SEMICN,"SEMICN"}, {COMMA,"COMMA"},
	{LPARENT,"LPARENT"}, {RPARENT,"RPARENT"}, {LBRACK,"LBRACK"}, {RBRACK,"RBRACK"}, {LBRACE,"LBRACE"}, {RBRACE,"RBRACE"}
};

static map<string, TokenType> reserveWords = {
	{"const", CONSTTK},
	{"int", INTTK},
	{"char", CHARTK},
	{"void", VOIDTK},
	{"main", MAINTK},
	{"if", IFTK},
	{"else", ELSETK},
	{"switch", SWITCHTK},
	{"case", CASETK},
	{"default", DEFAULTTK},
	{"while", WHILETK},
	{"for", FORTK},
	{"scanf", SCANFTK},
	{"printf", PRINTFTK},
	{"return", RETURNTK}
};


class Token;
ostream& operator<<(ostream& out, Token& A);

// Token Class
class Token {
private:
	string str;
	TokenType type;
public:
	friend ostream& operator<<(ostream& out, Token& A);
	Token();
	Token(string str, TokenType type);
	Token(char ch_i, TokenType type);
};

// IO File Streams
extern ifstream inputFile;
extern ofstream outputFile;

void getChar();
Token* getToken();
#endif // !_COMPILER_C0_LEXER
