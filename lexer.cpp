#include "lexer.h"

#include <utility>

// Some of the global variants
// See *Wolf Book* P71
static char ch;
static string token;
static int num;
static char store_ch;
static TokenType symbol;

// Some of the procedure and functions
static void getChar();
static void clearToken();
static bool isSpace();
static bool isNewline();
static bool isTab();
static bool isLetter();
static bool isDigit();

//static bool isIdenfr();
//static bool isIntcon();
//static bool isCharcon();
//static bool isStrcon();
//static bool isConsttk();
//static bool isInttk();
//static bool isChartk();
//static bool isVoidtk();
//static bool isMaintk();
//static bool isIftk();
//static bool isElsetk();
//static bool isSwitchtk();
//static bool isCasetk();
//static bool isDefaulttk();
//static bool isWhiletk();
//static bool isFortk();
//static bool isScanftk();
//static bool isPrintftk();
//static bool isReturntk();

// Character check
static bool isPlus();
static bool isMinu();
static bool isMult();
static bool isDiv();
static bool isLss();
//static bool isLeq();
static bool isGre();
//static bool isGeq();
//static bool isEql();
//static bool isNeq();
static bool isColon();
static bool isAssign();
static bool isSemicn();
static bool isComma();
static bool isLparent();
static bool isRparent();
static bool isLbrack();
static bool isRbrack();
static bool isLbrace();
static bool isRbrace();
static bool isApost();
static bool isChar();
static bool isQuota();
static bool isStringChar();
static bool isExclam();

// Token construct
static void catToken();
static void retract();
static TokenType reserver();
static int transNum();
static void error();



// Token Class Constructor
Token::Token()
= default;

Token::Token(string str, TokenType type) {
	this->str = std::move(str);
	this->type = type;
}

Token::Token(char ch_i, TokenType type) {
	this->str = ch_i;
	this->type = type;
}

// Token Class Output
ostream& operator<<(ostream& out, Token&& A) {
	out << tokenStringMap[A.type] << ' ' << A.str;
	return out;
}

// Some of the global variants
void getChar() {
	inputFile >> ch;
}

static void clearToken() {
	token = string("");
}

static bool isSpace() {
	return ch == ' ';
}

static bool isNewline() {
	return ch == '\n';
}

static bool isTab() {
	return ch == '\t';
}

static bool isLetter() {
	return (ch >= 'A' && ch <= 'Z') || // Uppercase
		(ch >= 'a' && ch <= 'z') || // Lowercase
		(ch == '_');
}

static bool isDigit() {
	return ch >= '0' && ch <= '9';
}

//static bool isIdenfr();
//static bool isIntcon();
//static bool isCharcon();
//static bool isStrcon();
//static bool isConsttk();
//static bool isInttk();
//static bool isChartk();
//static bool isVoidtk();
//static bool isMaintk();
//static bool isIftk();
//static bool isElsetk();
//static bool isSwitchtk();
//static bool isCasetk();
//static bool isDefaulttk();
//static bool isWhiletk();
//static bool isFortk();
//static bool isScanftk();
//static bool isPrintftk();
//static bool isReturntk();

static bool isPlus() {
	return ch == '+';
}

static bool isMinu() {
	return ch == '-';
}

static bool isMult() {
	return ch == '*';
}

static bool isDiv() {
	return ch == '/';
}

static bool isLss() {
	return ch == '<';
}

//static bool isLeq();

static bool isGre() {
	return ch == '>';
}

//static bool isGeq();
//static bool isEql();
//static bool isNeq();

static bool isColon() {
	return ch == ':';
}

static bool isAssign() {
	return ch == '=';
}

static bool isSemicn() {
	return ch == ';';
}

static bool isComma() {
	return ch == ',';
}

static bool isLparent() {
	return ch == '(';
}

static bool isRparent() {
	return ch == ')';
}

static bool isLbrack() {
	return ch == '[';
}

static bool isRbrack() {
	return ch == ']';
}

static bool isLbrace() {
	return ch == '{';
}

static bool isRbrace() {
	return ch == '}';
}

static bool isApost() {
	return ch == '\'';
}

static bool isChar() {
	return ch == '+' ||
		ch == '-' ||
		ch == '*' ||
		ch == '/' ||
		isDigit() ||
		isLetter();
}

static bool isQuota() {
	return ch == '"';
}

static bool isStringChar() {
	return !isQuota() && (ch >= 32 && ch <= 126);
}

static bool isExclam() {
	return ch == '!';
}

// Token Constructor
static void catToken() {
	token += ch;
}

static void retract() {
	inputFile.unget();
}

static TokenType reserver() {
	try
	{
		string lowercase("");
		transform(token.begin(), token.end(), lowercase.begin(),
			[](unsigned char c) { return tolower(c); });
		return reserveWords.at(lowercase);
	}
	catch (const out_of_range& e)
	{
		return IDENFR;
	}
}

static int transNum() {
	return stoi(token);
}

static void error() {
	cout << "error!" << endl;
}

Token getToken() {
	clearToken();
	while (isSpace() || isNewline() || isTab()) {
		getChar();
	}
	if (isLetter()) {
		while (isLetter() || isDigit())
		{
			catToken();
			getChar();
		}
		retract();
		return Token(token, reserver());
	}
	else if (isDigit()) {
		while (isDigit()) {
			catToken();
			getChar();
		}
		retract();
		num = transNum();
		return Token(token, INTCON);
	}
	else if (isApost()) {
		getChar();
		clearToken();
		if (isChar()) {
			catToken();
			getChar();
			if (isApost()) {
				return Token(token, CHARCON);
			}
		}
	}
	else if (isQuota()) {
		getChar();
		clearToken();
		while (isStringChar()) {
			catToken();
			getChar();
		}
		if (isQuota()) {
			return Token(token, STRCON);
		}
	}
	else if (isLss()) {
		getchar();
		catToken();
		if (isAssign()) {
			catToken();
			return Token(token, LEQ);
		}
		else {
			retract();
			return Token(token, LSS);
		}
	}
	else if (isGre()) {
		getchar();
		catToken();
		if (isAssign()) {
			catToken();
			return Token(token, GEQ);
		}
		else {
			retract();
			return Token(token, GRE);
		}
	}
	else if (isAssign()) {
		getchar();
		catToken();
		if (isAssign()) {
			catToken();
			return Token(token, EQL);
		}
		else {
			retract();
			return Token(token, ASSIGN);
		}
	}
	else if (isExclam()) {
		getchar();
		catToken();
		if (isAssign()) {
			catToken();
			return Token(token, NEQ);
		}
	}
	else if (isPlus()) {
		catToken();
		return Token(token, PLUS);
	}
	else if (isMinu()) {
		catToken();
		return Token(token, MINU);
	}
	else if (isMult()) {
		catToken();
		return Token(token, MULT);
	}
	else if (isDiv()) {
		catToken();
		return Token(token, DIV);
	}
	else if (isColon()) {
		catToken();
		return Token(token, COLON);
	}
	else if (isSemicn()) {
		catToken();
		return Token(token, SEMICN);
	}
	else if (isComma()) {
		catToken();
		return Token(token, COMMA);
	}
	else if (isLparent()) {
		catToken();
		return Token(token, LPARENT);
	}
	else if (isRparent()) {
		catToken();
		return Token(token, RPARENT);
	}
	else if (isLbrack()) {
		catToken();
		return Token(token, LBRACK);
	}
	else if (isRbrack()) {
		catToken();
		return Token(token, RBRACK);
	}
	else if (isLbrace()) {
		catToken();
		return Token(token, LBRACE);
	}
	else if (isRbrace()) {
		catToken();
		return Token(token, RBRACE);
	}
	else {
		error();
	}
	return Token();
}
