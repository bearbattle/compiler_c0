#include "lexer.h"
#include <cassert>
#include "SymbolTable.h"
// Parser will start with program()

void program(); // 11

// program:
static void constDec(); // 18
static void varDec(); // 21
static void returnFuncDef(); // 28
static void nonReturnFuncDef(); // 33
static void mainFunc(); // Native: compoundStat()

// constDec:
static void constDef(); // Native: int, <IDENFR>, integer(), char, character()

// varDec:
static void varDef(); // 25

// varDef:
static void varDefNoInit(); // Native: typeSpec(), <IDENFR>, [], unsignedInteger()
static void varDefWithInit(); // Native: typeSpec(), <IDENFR>, [], unsignedInteger(), constExpr()

// returnFuncDef:
static SymbolTableEntry *defHead(); // Native: int|char <IDENFR>
static void paramTable(); // 39
static void compoundStat(); // 41

// nonReturnFuncDef:
//static void defHead(); // Native: void <IDENFR>
//static void paramTable(); // 38
//static void compoundStat();

// paramTable: <IDENFR>
static void typeSpec(); // Native: int|char

// compoundStat:
//static void constDec();
//static void varDec();
static void stats(); // 46

// stats:
static void stat(); //49

// stat:
static void loopStat(); // 61
static void ifStat();// Native: condition(), stat()
static void returnCallStat();// 67
static void nonReturnCallStat();// 67
static void assignStat();// Native: <IDENFR>, expr()
static void readStat();// Native: scanf, <IDENFR>
static void writeStat();// Native: printf, <STRCON>, expr()
static void caseStat();// 70
static void returnStat();// Native: expr()
// { static void stats() } // 46

// loopStat:
//static void whileLoopStat(); // Native: condition(), stat()
//static void forLoopStat(); // Native: condition(), <IDENFR>, expr(), +|-, step()
static void condition(); // Native: expr(), relationOp()
static void step(); // Native: unsignedInteger()

// returnCallState, nonReturnCallState:
static void valueTable(); // Native: expr()

// caseStat: expr()
static void caseTable(); // Native: caseSubStat()
static void caseSubStat(); // Native: constExpr(), stat()
static void defaultStat(); // Native: stat()

// Common Usage
static void integer();// Native: +|- unsignedInteger()
static void unsignedInteger();// Native: <INTCON>
static void character(); // Native: <CHARCON>
static void expr(); // Native: +|-, term()
static void term(); // Native: factor(), * /
static void factor(); // Native: <IDENFR>, [], integer(), character(), returnCallStat()
static void relationOp(); // Native: < | <= | > | >= | != | ==
static void constExpr();

// error
static inline void error() {
    cout << "error!" << __FUNCTION__ << __LINE__ << endl;
}

// judge
static bool isVarDec();

enum VarDefType {
    VARDEFNOINIT,
    VARDEFWITHINIT,
    NONVARDEF
};

static VarDefType isVarDef();

enum StatType {
    LOOPSTAT,
    IFSTAT,
    RETURNCALLSTAT,
    NONRETURNCALLSTAT,
    ASSIGNSTAT,
    READSTAT,
    WRITESTAT,
    CASESTAT,
    RETURNSTAT,
    VOIDSTAT,
    STATS,
    NONSTAT
};

static StatType isStat();


static bool isCaseSubStat();

void program() {
    if (lexer::curToken == nullptr) {
        error();
    }
    TokenType curType = lexer::getTokenType();
    if (curType == CONSTTK) {
        constDec();
    }
    if (isVarDec()) {
        varDec();
    }
    while (curType == INTTK || curType == CHARTK || curType == VOIDTK) {
        if (curType == VOIDTK) {
            Token *next = lexer::preFetch(1);
            TokenType nextType = next->getType();
            if (nextType != MAINTK)
                nonReturnFuncDef();
        } else {
            returnFuncDef();
        }
        curType = lexer::getTokenType();
    }
    mainFunc();
    outputFile << "<程序>" << endl;
}

static void constDec() {
    assert (lexer::getTokenType() == CONSTTK);
    do {
        lexer::getToken();
        constDef();
        if (lexer::getTokenType() != SEMICN)
            error();
        lexer::getToken();
    } while (lexer::getTokenType() == CONSTTK);
    if (lexer::getTokenType() != SEMICN) {
        error();
    } else {
        outputFile << "<常量说明>" << endl;
    }
}

static bool isVarDec() {
    TokenType curType = lexer::getTokenType();
    if (curType == INTTK || curType == CHARTK) {
        Token *next = lexer::preFetch(2);
        TokenType nextType = next->getType();
        switch (nextType) {
            case SEMICN:
            case COMMA:
            case LBRACK:
            case ASSIGN:
                return true;
            default:
                break;
        }
    }
    return false;
}

static void varDec() {
    do {
        varDef();
        if (lexer::getTokenType() != SEMICN)
            error();
        lexer::getToken();
    } while (isVarDec());
    outputFile << "<常量说明>" << endl;
}


static void returnFuncDef() {
    auto *newEntry = defHead();
    if (lexer::getTokenType() == LPARENT) {
        lexer::getToken();
        paramTable();
        if (lexer::getTokenType() == RPARENT) {
            lexer::getToken();
            if (lexer::getTokenType() == LBRACE) {
                lexer::getToken();
                compoundStat();
                if (lexer::getTokenType() == RBRACE) {
                    lexer::getToken();
                    newEntry->setFunc(true);
                    getGSymTab().insert(make_pair(newEntry->getName(), newEntry));
                    outputFile << "<有返回值函数定义>" << endl;
                    return;
                }
            }
        }
    }
    error();
}

static void nonReturnFuncDef() {
    assert(lexer::getTokenType() == VOIDTK);
    lexer::getToken();
    if (lexer::getTokenType() == IDENFR) {
        const string &funcName = lexer::curToken->getStr();
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            paramTable();
            if (lexer::getTokenType() == RPARENT) {
                lexer::getToken();
                if (lexer::getTokenType() == LBRACE) {
                    lexer::getToken();
                    compoundStat();
                    if (lexer::getTokenType() == RBRACE) {
                        lexer::getToken();
                        outputFile << "<无返回值函数定义>" << endl;
                        auto *newEntry = new SymbolTableEntry(funcName, VOID);
                        newEntry->setFunc(true);
                        getGSymTab().insert(make_pair(funcName, newEntry));
                        return;
                    }
                }
            }
        }
    }
    error();
}

static void mainFunc() {
    if (lexer::getTokenType() == VOIDTK) {
        lexer::getToken();
        if (lexer::getTokenType() == IDENFR) {
            lexer::getToken();
            if (lexer::getTokenType() == LPARENT) {
                lexer::getToken();
                if (lexer::getTokenType() == RPARENT) {
                    lexer::getToken();
                    if (lexer::getTokenType() == LBRACE) {
                        lexer::getToken();
                        compoundStat();
                        if (lexer::getTokenType() == RBRACE) {
                            lexer::getToken();
                            outputFile << "<主函数>" << endl;
                            return;
                        }
                    }
                }
            }
        }
    }
}

static void constDef() {
    if (lexer::getTokenType() == INTTK) {
        lexer::getToken();
        if (lexer::getTokenType() == IDENFR) {
            lexer::getToken();
            if (lexer::getTokenType() == ASSIGN) {
                lexer::getToken();
                integer();
                while (lexer::getTokenType() == COMMA) {
                    lexer::getToken();
                    if (lexer::getTokenType() == IDENFR) {
                        lexer::getToken();
                        if (lexer::getTokenType() == ASSIGN) {
                            lexer::getToken();
                            integer();
                        }
                    }
                }
                outputFile << "<常量定义>" << endl;
                return;
            }
        }
    } else if (lexer::getTokenType() == CHARTK) {
        lexer::getToken();
        if (lexer::getTokenType() == IDENFR) {
            lexer::getToken();
            if (lexer::getTokenType() == ASSIGN) {
                lexer::getToken();
                character();
                while (lexer::getTokenType() == COMMA) {
                    lexer::getToken();
                    if (lexer::getTokenType() == IDENFR) {
                        lexer::getToken();
                        if (lexer::getTokenType() == ASSIGN) {
                            lexer::getToken();
                            integer();
                        }
                    }
                }
                outputFile << "<常量定义>" << endl;
                return;
            }
        }
    }
    error();
}

static VarDefType isVarDef() {
    if (lexer::getTokenType() == INTTK || lexer::getTokenType() == CHARTK) {
        Token *next2 = lexer::preFetch(2), *next5, *next8;
        TokenType nextType2 = next2->getType(), nextType5, nextType8;
        switch (nextType2) {
            case SEMICN:
            case COMMA:
                return VARDEFNOINIT;
            case ASSIGN:
                return VARDEFWITHINIT;
            case LBRACK:
                next5 = lexer::preFetch(5);
                nextType5 = next5->getType();
                switch (nextType5) {
                    case SEMICN:
                    case COMMA:
                        return VARDEFNOINIT;
                    case ASSIGN:
                        return VARDEFWITHINIT;
                    case LBRACK:
                        next8 = lexer::preFetch(8);
                        nextType8 = next8->getType();
                        switch (nextType8) {
                            case SEMICN:
                            case COMMA:
                                return VARDEFNOINIT;
                            case ASSIGN:
                                return VARDEFWITHINIT;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    return NONVARDEF;
}

static void varDef() {
    switch (isVarDef()) {
        case VARDEFNOINIT:
            varDefNoInit();
            break;
        case VARDEFWITHINIT:
            varDefWithInit();
            break;
        case NONVARDEF:
            error();
            break;
    }
    outputFile << "<变量说明>" << endl;
}

static void varDefNoInit() {
    assert(isVarDef() == VARDEFNOINIT);
    typeSpec();
    if (lexer::getTokenType() == IDENFR) {
        lexer::getToken();
        if (lexer::getTokenType() == LBRACK) {
            lexer::getToken();
            unsignedInteger();
            if (lexer::getTokenType() == RBRACK) {
                lexer::getToken();
                if (lexer::getTokenType() == LBRACK) {
                    lexer::getToken();
                    unsignedInteger();
                    if (lexer::getTokenType() == RBRACK) {
                        lexer::getToken();
                    } else {
                        error();
                    }
                }
            } else {
                error();
            }
        }
        while (lexer::getTokenType() == COMMA) {
            if (lexer::getTokenType() == IDENFR) {
                lexer::getToken();
                if (lexer::getTokenType() == LBRACK) {
                    lexer::getToken();
                    unsignedInteger();
                    if (lexer::getTokenType() == RBRACK) {
                        lexer::getToken();
                        if (lexer::getTokenType() == LBRACK) {
                            lexer::getToken();
                            unsignedInteger();
                            if (lexer::getTokenType() == RBRACK) {
                                lexer::getToken();
                            } else {
                                error();
                            }
                        }
                    } else {
                        error();
                    }
                }
            }
        }
    }
    outputFile << "<变量定义无初始化>" << endl;
}

static void varDefWithInit() {
    assert(isVarDef() == VARDEFWITHINIT);
    typeSpec();
    if (lexer::getTokenType() == IDENFR) { // Dimension 0 Left
        lexer::getToken();
        if (lexer::getTokenType() == LBRACK) {
            lexer::getToken();
            unsignedInteger();
            if (lexer::getTokenType() == RBRACK) {// Dimension 1 Left
                lexer::getToken();
                if (lexer::getTokenType() == LBRACK) {
                    lexer::getToken();
                    unsignedInteger();
                    if (lexer::getTokenType() == RBRACK) { // Dimension 2 Left
                        lexer::getToken();
                        if (lexer::getTokenType() == ASSIGN) { // Dimension 2 Right Begin
                            lexer::getToken();
                            if (lexer::getTokenType() == LBRACE) { // Outer Brace
                                lexer::getToken();
                                if (lexer::getTokenType() == LBRACE) { // Inner Brace
                                    lexer::getToken();
                                    constExpr();
                                    while (lexer::getTokenType() == COMMA) {
                                        lexer::getToken();
                                        constExpr();
                                    }
                                    if (lexer::getTokenType() == RBRACE) {
                                        lexer::getToken();
                                    } else {
                                        error();
                                    }
                                } else {
                                    error();
                                }
                                while (lexer::getTokenType() == COMMA) {
                                    lexer::getToken();
                                    if (lexer::getTokenType() == LBRACE) { // Inner Brace
                                        lexer::getToken();
                                        constExpr();
                                        while (lexer::getTokenType() == COMMA) {
                                            lexer::getToken();
                                            constExpr();
                                        }
                                        if (lexer::getTokenType() == RBRACE) {
                                            lexer::getToken();
                                        } else {
                                            error();
                                        }
                                    } else {
                                        error();
                                    }
                                }
                            } else {
                                error();
                            }
                            // Dimension 2 Right End
                        }
                    } else {
                        error();
                    }
                } else if (lexer::getTokenType() == ASSIGN) { // Dimension 1 Right
                    lexer::getToken();
                    if (lexer::getTokenType() == LBRACE) {
                        constExpr();
                        while (lexer::getTokenType() == COMMA) {
                            lexer::getToken();
                            constExpr();
                        }
                        lexer::getToken();
                        if (lexer::getTokenType() == RBRACE) {
                            lexer::getToken();
                        }
                    }

                }
            } else {
                error();
            }
        } else if (lexer::getTokenType() == ASSIGN) { // Dimension 0 Right
            lexer::getToken();
            constExpr();
        } else {
            error();
        }
    }
    outputFile << "<变量定义及初始化>" << endl;
}

static SymbolTableEntry *defHead() {
    TokenType retType = lexer::getTokenType();
    if (retType == INTTK || retType == CHARTK) {
        lexer::getToken();
        if (lexer::getTokenType() == IDENFR) {
            const string &funcName = lexer::curToken->getStr();
            const BaseType baseType = retType == INTTK ? INT : CHAR;
            lexer::getToken();
            outputFile << "<声明头部>" << endl;
            return new SymbolTableEntry(funcName, baseType);
        }
    } else {
        error();
    }
    return nullptr;
}

static void paramTable() {
    if (lexer::getTokenType() == RPARENT) {
        outputFile << "<参数表>" << endl;
        return;
    } else if (lexer::getTokenType() == INTTK || lexer::getTokenType() == CHARTK) {
        typeSpec();
        if (lexer::getTokenType() == IDENFR) {
            lexer::getToken();
            while (lexer::getTokenType() == COMMA) {
                lexer::getToken();
                typeSpec();
                if (lexer::getTokenType() == IDENFR) {
                    lexer::getToken();
                } else {
                    error();
                }
            }
            outputFile << "<参数表>" << endl;
            return;
        } else {
            error();
        }
    } else {
        error();
    }
}

static void compoundStat() {
    if (lexer::getTokenType() == CONSTTK) {
        constDec();
    }
    if (isVarDec()) {
        varDec();
    }
    stats();
    outputFile << "<复合语句>" << endl;
}

static void typeSpec() {
    if (lexer::getTokenType() == INTTK || lexer::getTokenType() == CHARTK) {
        return;
    } else {
        error();
    }
}

static void stats() {
    while (isStat() != NONSTAT) {
        stat();
    }
    outputFile << "<语句列>" << endl;
}

static StatType isStat() {
    Token *next;
    TokenType nextType;
    BaseType retType;
    string id;
    SymbolTableEntry *ptr;
    switch (lexer::getTokenType()) {
        case WHILETK:
        case FORTK:
            return LOOPSTAT;
        case IFTK:
            return IFSTAT;
        case SCANFTK:
            return READSTAT;
        case PRINTFTK:
            return WRITESTAT;
        case SWITCHTK:
            return CASESTAT;
        case SEMICN:
            return VOIDSTAT;
        case RETURNTK:
            return RETURNSTAT;
        case LBRACE:
            return STATS;
        case IDENFR:
            id = lexer::curToken->getStr();
            next = lexer::preFetch(1);
            nextType = next->getType();
            if (nextType == ASSIGN) {
                return ASSIGNSTAT;
            } else if (nextType == LPARENT) {
                ptr = getGSymTab().at(id);
                if (ptr->isFunc()) {
                    retType = ptr->getBaseType();
                    if (retType == VOID) {
                        return NONRETURNCALLSTAT;
                    } else if (retType == INT || retType == CHAR) {
                        return RETURNCALLSTAT;
                    }
                } else {
                    error();
                }
            } else {
                error();
            }
        default:
            return NONSTAT;
    }
}

static void stat() {
    switch (isStat()) {
        case LOOPSTAT:
            loopStat();
            break;
        case IFSTAT:
            ifStat();
            break;
        case RETURNCALLSTAT:
            returnCallStat();
            break;
        case NONRETURNCALLSTAT:
            nonReturnCallStat();
            break;
        case ASSIGNSTAT:
            assignStat();
            break;
        case READSTAT:
            readStat();
            break;
        case WRITESTAT:
            writeStat();
            break;
        case CASESTAT:
            caseStat();
            break;
        case RETURNSTAT:
            returnStat();
            break;
        case VOIDSTAT:
            break;
        case STATS:
            if (lexer::getTokenType() == LBRACE) {
                lexer::getToken();
                stats();
                if (lexer::getTokenType() == RBRACE) {
                    lexer::getToken();
                } else {
                    error();
                }
            } else {
                error();
            }
            break;
        default:
            error();
            break;
    }
    outputFile << "<语句>" << endl;
}

static void loopStat() {
    if (lexer::getTokenType() == WHILETK) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            condition();
            if (lexer::getTokenType() == RPARENT) {
                stat();
                outputFile << "<循环语句>" << endl;
                return;
            }
        }
    } else if (lexer::getTokenType() == FORTK) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            if (lexer::getTokenType() == IDENFR) {
                lexer::getToken();
                if (lexer::getTokenType() == ASSIGN) {
                    lexer::getToken();
                    expr();
                    if (lexer::getTokenType() == SEMICN) {
                        lexer::getToken();
                        condition();
                        if (lexer::getTokenType() == SEMICN) {
                            lexer::getToken();
                            if (lexer::getTokenType() == IDENFR) {
                                lexer::getToken();
                                if (lexer::getTokenType() == ASSIGN) {
                                    lexer::getToken();
                                    if (lexer::getTokenType() == IDENFR) {
                                        lexer::getToken();
                                        if (lexer::getTokenType() == PLUS ||
                                            lexer::getTokenType() == MINU) {
                                            lexer::getToken();
                                            step();
                                            if (lexer::getTokenType() == RPARENT) {
                                                lexer::getToken();
                                                stat();
                                                outputFile << "<循环语句>" << endl;
                                                return;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    error();
}

static void ifStat() {
    if (lexer::getTokenType() == IFTK) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            condition();
            if (lexer::getTokenType() == RPARENT) {
                lexer::getToken();
                stat();
                if (lexer::getTokenType() == ELSETK) {
                    lexer::getToken();
                    stat();
                }
                outputFile << "<条件语句>" << endl;
                return;
            }
        }
    }
    error();
}

static void returnCallStat() {
    if (lexer::getTokenType() == IDENFR) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            valueTable();
            if (lexer::getTokenType() == RPARENT) {
                lexer::getToken();
                outputFile << "<有返回值函数调用语句>" << endl;
                return;
            }
        }
    }
    error();
}

static void nonReturnCallStat() {
    if (lexer::getTokenType() == IDENFR) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            valueTable();
            if (lexer::getTokenType() == RPARENT) {
                lexer::getToken();
                outputFile << "<无返回值函数调用语句>" << endl;
                return;
            }
        }
    }
    error();
}

static void assignStat() {
    if (lexer::getTokenType() == IDENFR) { // Dimension 0 Left
        lexer::getToken();
        if (lexer::getTokenType() == LBRACK) {
            lexer::getToken();
            expr();
            if (lexer::getTokenType() == RBRACK) {// Dimension 1 Left
                lexer::getToken();
                if (lexer::getTokenType() == LBRACK) {
                    lexer::getToken();
                    expr();
                    if (lexer::getTokenType() == RBRACK) { // Dimension 2 Left
                        lexer::getToken();
                        if (lexer::getTokenType() == ASSIGN) { // Dimension 2 Right
                            lexer::getToken();
                            expr();
                            outputFile << "<赋值语句>" << endl;
                            return;
                        }
                    }
                } else if (lexer::getTokenType() == ASSIGN) { // Dimension 1 Right
                    lexer::getToken();
                    expr();
                    outputFile << "<赋值语句>" << endl;
                    return;
                }
            }
        } else if (lexer::getTokenType() == ASSIGN) { // Dimension 0 Right
            lexer::getToken();
            expr();
            outputFile << "<赋值语句>" << endl;
            return;
        }
    }
    error();
}

static void readStat() {
    if (lexer::getTokenType() == SCANFTK) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            if (lexer::getTokenType() == IDENFR) {
                lexer::getToken();
                if (lexer::getTokenType() == RPARENT) {
                    lexer::getToken();
                    outputFile << "<读语句>" << endl;
                    return;
                }
            }
        }
    }
    error();
}

static void writeStat() {
    if (lexer::getTokenType() == PRINTFTK) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            if (lexer::getTokenType() == STRCON) {
                lexer::getToken();
                if (lexer::getTokenType() == COMMA) {
                    lexer::getToken();
                } else if (lexer::getTokenType() == RPARENT) {
                    lexer::getToken();
                    outputFile << "<写语句>" << endl;
                    return;
                }
            }
            expr();
            if (lexer::getTokenType() == RPARENT) {
                lexer::getToken();
                outputFile << "<写语句>" << endl;
                return;
            }
        }
    }
    error();
}

static void caseStat() {
    if (lexer::getTokenType() == SWITCHTK) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            expr();
            if (lexer::getTokenType() == RPARENT) {
                lexer::getToken();
                if (lexer::getTokenType() == LBRACE) {
                    lexer::getToken();
                    caseTable();
                    defaultStat();
                    if (lexer::getTokenType() == RBRACE) {
                        lexer::getToken();
                        outputFile << "<情况语句>" << endl;
                        return;
                    }
                }
            }
        }
    }
    error();
}

static void returnStat() {
    if (lexer::getTokenType() == RETURNTK) {
        lexer::getToken();
        if (lexer::getTokenType() == LPARENT) {
            lexer::getToken();
            expr();
            if (lexer::getTokenType() == RPARENT) {
                lexer::getToken();
                outputFile << "<返回语句>" << endl;
                return;
            } else {
                error();
            }
        }
        outputFile << "<返回语句>" << endl;
        return;
    }
    error();
}

static void condition() {
    expr();
    relationOp();
    expr();
    outputFile << "<条件>" << endl;
}

static void step() {
    unsignedInteger();
    outputFile << "<步长>" << endl;
}

static void valueTable() {
    if (lexer::getTokenType() == RPARENT) {
        outputFile << "<值参数表>" << endl;
        return;
    }
    expr();
    while (lexer::getTokenType() == COMMA) {
        lexer::getToken();
        expr();
    }
    outputFile << "<值参数表>" << endl;
}

static void caseTable() {
    do {
        caseSubStat();
    } while (isCaseSubStat());
    outputFile << "<情况表>" << endl;
}

static bool isCaseSubStat() {
    return lexer::getTokenType() == CASETK;
}

static void caseSubStat() {
    if (isCaseSubStat()) {
        lexer::getToken();
        constExpr();
        if (lexer::getTokenType() == COLON) {
            lexer::getToken();
            stat();
        }
    }
    outputFile << "<情况子语句>" << endl;
}

static void defaultStat() {
    if (lexer::getTokenType() == DEFAULTTK) {
        lexer::getToken();
        if (lexer::getTokenType() == COLON) {
            lexer::getToken();
            stat();
        }
    }
    outputFile << "<缺省>" << endl;
}

static void integer() {
    if (lexer::getTokenType() == PLUS ||
        lexer::getTokenType() == MINU) {
        lexer::getToken();
        unsignedInteger();
        outputFile << "<整数>" << endl;
        return;
    }
    error();
}

static void unsignedInteger() {
    if (lexer::getTokenType() == INTCON) {
        lexer::getToken();
        outputFile << "<无符号整数>" << endl;
    } else {
        error();
    }
}

static void character() {
    if (lexer::getTokenType() == CHARCON) {
        lexer::getToken();
    } else {
        error();
    }
}

static void expr() {
    if (lexer::getTokenType() == PLUS ||
        lexer::getTokenType() == MINU) {
        lexer::getToken();
    }
    term();
    while (lexer::getTokenType() == PLUS ||
           lexer::getTokenType() == MINU) {
        lexer::getToken();
        term();
    }
    outputFile << "<表达式>" << endl;
}

static void term() {
    factor();
    while (lexer::getTokenType() == MULT ||
           lexer::getTokenType() == DIV) {
        lexer::getToken();
        term();
    }
    outputFile << "<项>" << endl;
}

static void factor() {
    if (isStat() == RETURNCALLSTAT) {
        returnCallStat();
        outputFile << "<因子>" << endl;
        return;
    }
    if (lexer::getTokenType() == IDENFR) { // Dimension 0 Left
        lexer::getToken();
        if (lexer::getTokenType() == LBRACK) {
            lexer::getToken();
            expr();
            if (lexer::getTokenType() == RBRACK) {// Dimension 1 Left
                lexer::getToken();
                if (lexer::getTokenType() == LBRACK) {
                    lexer::getToken();
                    expr();
                    if (lexer::getTokenType() == RBRACK) { // Dimension 2 Left
                        lexer::getToken();
                        outputFile << "<因子>" << endl;
                        return;
                    } else {
                        error();
                    }
                }
                outputFile << "<因子>" << endl;
                return;
            } else {
                error();
            }
        }
        outputFile << "<因子>" << endl;
        return;
    } else if (lexer::getTokenType() == PLUS ||
               lexer::getTokenType() == MINU ||
               lexer::getTokenType() == INTCON) {
        integer();
        outputFile << "<因子>" << endl;
        return;
    } else if (lexer::getTokenType() == CHARCON) {
        character();
        outputFile << "<因子>" << endl;
        return;
    } else if (lexer::getTokenType() == LPARENT) {
        lexer::getToken();
        expr();
        if (lexer::getTokenType() == RPARENT) {
            lexer::getToken();
            outputFile << "<因子>" << endl;
            return;
        }
    }
}

static void relationOp() {
    switch (lexer::getTokenType()) {
        case LSS:
        case LEQ:
        case GRE:
        case GEQ:
        case EQL:
        case NEQ:
            lexer::getToken();
            return;
        default:
            error();
    }
}

static void constExpr() {
    if (lexer::getTokenType() == INTCON ||
        lexer::getTokenType() == CHARCON) {
        integer();
        outputFile << "<常量>" << endl;
        return;
    }
}