#ifndef COMPILER_0_TOKEN_H
#define COMPILER_0_TOKEN_H

#include <string>
#include <map>

using namespace std;

// Defines the types of token
enum TokenType {
    INVALID,
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
extern map<TokenType, string> tokenStringMap;

extern map<string, TokenType> reserveWords;


class Token;

ostream &operator<<(ostream &out, const Token &A);

// Token Class
class Token {
private:
    string str;
    TokenType type;
    int line;

public:
    friend ostream &operator<<(ostream &out, const Token &A);

    Token();

    Token(string str, TokenType type);

    Token(char ch_i, TokenType type);

    TokenType getType();

    int getLine() const;

    const string &getStr() const;
};


#endif //COMPILER_0_TOKEN_H
