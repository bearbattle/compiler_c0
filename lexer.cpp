#include "lexer.h"

#include <utility>
#include <deque>

// Some of the global variants
// See *Wolf Book* P71
static char ch;
static string token;
static int num;
static char store_ch;
static TokenType symbol;

// Some of the procedure and functions
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

static bool isEnd();

// Token construct
static void catToken();

static void retract();

static TokenType reserved();

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
ostream& operator<<(ostream & out, const Token & A) {
    out << tokenStringMap[A.type] << ' ' << A.str;
    return out;
}

TokenType Token::getType() {
    return this->type;
}

const string& Token::getStr() const {
    return str;
}

// Some of the global variants
void getChar() {
    ch = inputFile.get();
}

static void clearToken() {
    token = string("");
}

static bool isSpace() {
    return ch == ' ';
}

static bool isNewline() {
    return ch == '\n' ||
        ch == '\r';
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

static bool isEnd() {
    return ch == EOF;
}

// Token Constructor
static void catToken() {
    token += ch;
}

static void retract() {
    inputFile.unget();
}

static TokenType reserved() {

    string lowercase;
    for (char c : token) {
        lowercase += tolower(c);
    }
    auto item = reserveWords.find(lowercase);

    if (item != reserveWords.end()) {
        return item->second;
    }
    else {
        return IDENFR;
    }

}

static int transNum() {
    return stoi(token);
}

static inline void error() {
    cout << "Lexer Error!" << endl;
}

Token* lexer::_getToken() {
    getChar();
    clearToken();
    while (isSpace() || isNewline() || isTab()) {
        getChar();
    }
    if (isEnd()) {
        return nullptr;
    }
    if (isLetter()) {
        while (isLetter() || isDigit()) {
            catToken();
            getChar();
        }
        retract();
        return new Token(token, reserved());
    }
    else if (isDigit()) {
        while (isDigit()) {
            catToken();
            getChar();
        }
        retract();
        num = transNum();
        return new Token(token, INTCON);
    }
    else if (isApost()) {
        getChar();
        clearToken();
        if (isChar()) {
            catToken();
            getChar();
            if (isApost()) {
                return new Token(token, CHARCON);
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
            return new Token(token, STRCON);
        }
    }
    else if (isLss()) {
        catToken();
        getChar();
        if (isAssign()) {
            catToken();
            return new Token(token, LEQ);
        }
        else {
            retract();
            return new Token(token, LSS);
        }
    }
    else if (isGre()) {
        catToken();
        getChar();
        if (isAssign()) {
            catToken();
            return new Token(token, GEQ);
        }
        else {
            retract();
            return new Token(token, GRE);
        }
    }
    else if (isAssign()) {
        catToken();
        getChar();
        if (isAssign()) {
            catToken();
            return new Token(token, EQL);
        }
        else {
            retract();
            return new Token(token, ASSIGN);
        }
    }
    else if (isExclam()) {
        catToken();
        getChar();
        if (isAssign()) {
            catToken();
            return new Token(token, NEQ);
        }
    }
    else if (isPlus()) {
        catToken();
        return new Token(token, PLUS);
    }
    else if (isMinu()) {
        catToken();
        return new Token(token, MINU);
    }
    else if (isMult()) {
        catToken();
        return new Token(token, MULT);
    }
    else if (isDiv()) {
        catToken();
        return new Token(token, DIV);
    }
    else if (isColon()) {
        catToken();
        return new Token(token, COLON);
    }
    else if (isSemicn()) {
        catToken();
        return new Token(token, SEMICN);
    }
    else if (isComma()) {
        catToken();
        return new Token(token, COMMA);
    }
    else if (isLparent()) {
        catToken();
        return new Token(token, LPARENT);
    }
    else if (isRparent()) {
        catToken();
        return new Token(token, RPARENT);
    }
    else if (isLbrack()) {
        catToken();
        return new Token(token, LBRACK);
    }
    else if (isRbrack()) {
        catToken();
        return new Token(token, RBRACK);
    }
    else if (isLbrace()) {
        catToken();
        return new Token(token, LBRACE);
    }
    else if (isRbrace()) {
        catToken();
        return new Token(token, RBRACE);
    }
    else {
        error();
    }
    return new Token();
}

Token* lexer::curToken = nullptr;

deque<Token*> lexer::afterWards;

TokenType lexer::getTokenType() {
    return lexer::curToken->getType();
}

void lexer::getToken() {
    if (curToken != nullptr) {
        outputFile << *curToken << endl;
    }
    if (!afterWards.empty()) {
        curToken = afterWards.front();
        afterWards.pop_front();
    }
    else {
        curToken = _getToken();
    }
}

Token* lexer::preFetch(int amount) {
    if (afterWards.size() >= amount) {
        return afterWards[amount - 1];
    }
    for (int i = afterWards.size(); i < amount && !isEnd(); ++i) {
        Token* ptk = _getToken();
        if (ptk != nullptr) {
            afterWards.push_back(ptk);
        }
        else {
            break;
        }
    }
    return afterWards[amount - 1];
}
