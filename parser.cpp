#include "lexer.h"
#include <cassert>
#include "SymbolTable.h"
#include "Error.h"
#include "MidCode.h"
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
static SymTabEntry* defHead(); // Native: int|char <IDENFR>
static void paramTable(); // 39
static void compoundStat(); // 41
static ParamTab* curParamTab;

// nonReturnFuncDef:
//static void defHead(); // Native: void <IDENFR>
//static void paramTable(); // 38
//static void compoundStat();

// paramTable: <IDENFR>
static BaseType typeSpec(); // Native: int|char

// compoundStat:
//static void constDec();
//static void varDec();
static void stats(); // 46

// stats:
static void stat(); //49

// stat:
static void loopStat(); // 61
static void ifStat();// Native: condition(), stat()
static BaseType returnCallStat();// 67
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
static void condition(VarBase*& leftVar, MidOp& op, VarBase*& rightVar); // Native: expr(), relationOp()
static int step(); // Native: unsignedInteger()

// returnCallState, nonReturnCallState:
static void valueTable(ParamTab* ptr_paramTab, CallMid* callMid); // Native: expr()

// caseStat: expr()
static void caseTable(BaseType caseType, SwitchLabel switchLabel); // Native: caseSubStat()
static void caseSubStat(BaseType caseType, SwitchLabel switchLabel); // Native: constExpr(), stat()
static void defaultStat(SwitchLabel switchLabel); // Native: stat()

// Common Usage
static int integer();// Native: +|- unsignedInteger()
static int unsignedInteger();// Native: <INTCON>
static char character(); // Native: <CHARCON>
static string stringCon();

static BaseType expr(VarBase*& var); // Native: +|-, term()
static BaseType term(VarBase*& var); // Native: factor(), * /
static BaseType factor(VarBase*& var); // Native: <IDENFR>, [], integer(), character(), returnCallStat()
static MidOp relationOp(); // Native: < | <= | > | >= | != | ==
static BaseType constExpr(VarBase*& var);

// error
#define parseError() do { \
    cout << "Parser Error! At " << __FUNCTION__ << ":" << __LINE__ << endl; \
} while (false)

#define ACCEPT(tokenType) do { \
    if (lexer::getTokenType() == tokenType) { \
        lexer::getToken();     \
    } else {                   \
        cout << "should be" << tokenType << endl;  \
        parseError();          \
    }                      \
} while (false)

// judge

static bool isVarDec();

enum VarDefType
{
    VARDEFNOINIT,
    VARDEFWITHINIT,
    NONVARDEF
};

static VarDefType isVarDef();

enum StatType
{
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

// SymbolTableLayer
static int curLayer = 0;

void program()
{
    if (lexer::curToken == nullptr)
    {
        parseError();
    }
    if (lexer::getTokenType() == CONSTTK)
    {
        constDec();
    }
    if (isVarDec())
    {
        varDec();
    }
    while (lexer::getTokenType() == INTTK ||
        lexer::getTokenType() == CHARTK ||
        lexer::getTokenType() == VOIDTK)
    {
        if (lexer::getTokenType() == VOIDTK)
        {
            Token* next = lexer::preFetch(1);
            TokenType nextType = next->getType();
            if (nextType != MAINTK)
            {
                nonReturnFuncDef();
            }
            else
            {
                break;
            }
        }
        else
        {
            returnFuncDef();
        }
    }
    mainFunc();
    outputFile << "<程序>" << endl;
}

#define SemicnExpected errList.emplace_back(k, lexer::prevToken->getLine())
#define RparentExpected errList.emplace_back(l, lexer::prevToken->getLine())
#define RbrackExpected errList.emplace_back(m, lexer::prevToken->getLine())

static void constDec()
{
    assert(lexer::getTokenType() == CONSTTK);
    do
    {
        lexer::getToken();
        constDef();
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
            parseError();
        }
    } while (lexer::getTokenType() == CONSTTK);
    outputFile << "<常量说明>" << endl;
}

static bool isVarDec()
{
    TokenType curType = lexer::getTokenType();
    if (curType == INTTK || curType == CHARTK)
    {
        Token* next = lexer::preFetch(2);
        TokenType nextType = next->getType();
        switch (nextType)
        {
        case LPARENT:
            return false;
        default:
            return true;
        }
    }
    return false;
}

static void varDec()
{
    do
    {
        varDef();
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
            parseError();
        }
    } while (isVarDec());
    outputFile << "<变量说明>" << endl;
}


#define SymTabAddFunc do { \
newEntry->setFunc(true);   \
if (getGSymTab()->exist(newEntry->getName())) { \
errList.emplace_back(b, lexer::prevToken);      \
curParamTab = new ParamTab();                   \
override = true;           \
} else {                   \
getGSymTab()->addEntry(newEntry->getName(), newEntry); \
}} while (false)

// Error h:
static bool isReturnFunc = false;
static int returnCount = 0;
static BaseType returnType = UNDEF;

static void returnFuncDef()
{
    bool override = false;
    SymTabs.push_back(new SymTab(++curLayer));
    auto* newEntry = defHead();
    SymTabAddFunc;
    // Error h:
    isReturnFunc = true;
    returnCount = 0;
    returnType = newEntry->getBaseType();
    ACCEPT(LPARENT);
    if (!override)
    {
        curParamTab = new ParamTab();
    }
    paramTable();
    newEntry->setParamTab(curParamTab);
    if (lexer::getTokenType() != RPARENT)
    {
        RparentExpected;
        goto Rparent;
    }
    lexer::getToken();
Rparent:
    midCodes.push_back(new FunctionMid(newEntry));
    ACCEPT(LBRACE);
    compoundStat();
    ACCEPT(RBRACE);
    outputFile << "<有返回值函数定义>" << endl;
    newEntry->setSymTab(SymTabs[curLayer--]);
    SymTabs.pop_back();
    if (returnCount == 0)
    {
        errList.emplace_back(h, lexer::prevToken);
    }
    isReturnFunc = false;
    return;
    midCodes.push_back(new FunctionEndMid());
    parseError();
}

static bool isNonReturnFunc;

static void nonReturnFuncDef()
{
    bool override = false;
    isNonReturnFunc = true;
    assert(lexer::getTokenType() == VOIDTK);
    SymTabs.push_back(new SymTab(++curLayer));
    lexer::getToken();
    if (lexer::getTokenType() == IDENFR)
    {
        const string& funcName = lexer::curToken->getStr();
        auto* newEntry = new SymTabEntry(funcName, VOID, lexer::curToken->getLine());
        SymTabAddFunc;
        lexer::getToken();
        ACCEPT(LPARENT);
        if (!override)
        {
            curParamTab = new ParamTab();
        }
        paramTable();
        newEntry->setParamTab(curParamTab);
        if (lexer::getTokenType() != RPARENT)
        {
            RparentExpected;
            goto Rparent;
        }
        lexer::getToken();
    Rparent:
        midCodes.push_back(new FunctionMid(newEntry));
        ACCEPT(LBRACE);
        compoundStat();
        ACCEPT(RBRACE);
        outputFile << "<无返回值函数定义>" << endl;
        newEntry->setSymTab(SymTabs[curLayer--]);
        SymTabs.pop_back();
        isNonReturnFunc = false;
        midCodes.push_back(new FunctionEndMid());
        return;
    }
    parseError();
}

static void mainFunc()
{
    assert(curLayer == 0);
    SymTabEntry* mainEntry;
    ACCEPT(VOIDTK);
    if (lexer::getTokenType() == MAINTK)
    {
        // Add SymTab of void main()
        mainEntry = new
            SymTabEntry("main", VOID,
                lexer::curToken->getLine());
        getGSymTab()->addEntry("main", mainEntry);
        SymTabs.push_back(new SymTab(++curLayer));
        isNonReturnFunc = true;
        lexer::getToken();
        ACCEPT(LPARENT);
        if (lexer::getTokenType() != RPARENT)
        {
            RparentExpected;
            goto Rparent;
        }
        lexer::getToken();
    Rparent:
        mainEntry->setParamTab(new ParamTab());
        mainEntry->setFunc(true);
        midCodes.push_back(new FunctionMid(mainEntry));
        ACCEPT(LBRACE);
        compoundStat();
        ACCEPT(RBRACE);
        outputFile << "<主函数>" << endl;
        mainEntry->setSymTab(SymTabs[curLayer--]);
        SymTabs.pop_back();
        isNonReturnFunc = false;
        midCodes.push_back(new FunctionEndMid());
        return;
    }
}

#define SymTabAddConst do { \
    constEntry = new \
        SymTabEntry(lexer::curToken->getStr(), constType, \
            lexer::curToken->getLine(), curLayer == 0);                  \
    constEntry->setConst(true);                           \
    SymTabs[curLayer]->addEntry(lexer::curToken->getStr(), \
        constEntry); } while (false)

static void constDef()
{
    BaseType constType = lexer::getTokenType() == INTTK ? INT : CHAR;
    SymTabEntry* constEntry;
    if (lexer::getTokenType() == INTTK)
    {
        lexer::getToken();
        if (lexer::getTokenType() == IDENFR)
        {
            SymTabAddConst;
            lexer::getToken();
            ACCEPT(ASSIGN);
            constEntry->setInitVal(integer());
            while (lexer::getTokenType() == COMMA)
            {
                lexer::getToken();
                if (lexer::getTokenType() == IDENFR)
                {
                    SymTabAddConst;
                    lexer::getToken();
                    ACCEPT(ASSIGN);
                    constEntry->setInitVal(integer());
                }
            }
            outputFile << "<常量定义>" << endl;
            return;
        }
    }
    else if (lexer::getTokenType() == CHARTK)
    {
        lexer::getToken();
        if (lexer::getTokenType() == IDENFR)
        {
            SymTabAddConst;
            lexer::getToken();
            ACCEPT(ASSIGN);
            constEntry->setInitVal(character());
            while (lexer::getTokenType() == COMMA)
            {
                lexer::getToken();
                if (lexer::getTokenType() == IDENFR)
                {
                    SymTabAddConst;
                    lexer::getToken();
                    ACCEPT(ASSIGN);
                    constEntry->setInitVal(character());
                }
            }
            outputFile << "<常量定义>" << endl;
            return;
        }
    }
    parseError();
}

static VarDefType isVarDef()
{
    if (lexer::getTokenType() == INTTK || lexer::getTokenType() == CHARTK)
    {
        int i = 2;
        TokenType nextType = lexer::preFetch(i)->getType();
        while (true)
        {
            switch (nextType)
            {
            case ASSIGN:
                return VARDEFWITHINIT;
            case LBRACK:
            case INTCON:
            case RBRACK:
                break;
            default:
                return VARDEFNOINIT;
            }
            nextType = lexer::preFetch(++i)->getType();
        }
    }
    return NONVARDEF;
}

static void varDef()
{
    switch (isVarDef())
    {
    case VARDEFNOINIT:
        varDefNoInit();
        break;
    case VARDEFWITHINIT:
        varDefWithInit();
        break;
    case NONVARDEF:
        parseError();
        break;
    }
    outputFile << "<变量定义>" << endl;
}

#define SymTabAddVar do { \
    varEntry = new \
        SymTabEntry(lexer::curToken->getStr(), varType, \
            lexer::curToken->getLine(), curLayer == 0); \
    SymTabs[curLayer]->addEntry(lexer::curToken->getStr(), \
        varEntry); } while (false)

#define InitVarArray(dimension) do { \
varEntry->setDimension(dimension);\
varEntry->setLength(dimension, unsignedInteger());} while(false)

static void varDefNoInit()
{
    assert(isVarDef() == VARDEFNOINIT);
    BaseType varType = typeSpec();
    SymTabEntry* varEntry;
    if (lexer::getTokenType() == IDENFR)
    {
        SymTabAddVar;
        lexer::getToken();
        if (lexer::getTokenType() == LBRACK)
        {
            lexer::getToken();
            InitVarArray(1);
            if (lexer::getTokenType() != RBRACK)
            {
                RbrackExpected;
                parseError();
                goto Rbrack1;
            }
            lexer::getToken();
        Rbrack1:
            if (lexer::getTokenType() == LBRACK)
            {
                lexer::getToken();
                InitVarArray(2);
                if (lexer::getTokenType() != RBRACK)
                {
                    RbrackExpected;
                    parseError();
                    goto Rbrack2;
                }
                lexer::getToken();
            Rbrack2:;
            }
        }
        while (lexer::getTokenType() == COMMA)
        {
            lexer::getToken();
            if (lexer::getTokenType() == IDENFR)
            {
                SymTabAddVar;
                lexer::getToken();
                if (lexer::getTokenType() == LBRACK)
                {
                    lexer::getToken();
                    InitVarArray(1);
                    if (lexer::getTokenType() != RBRACK)
                    {
                        RbrackExpected;
                        parseError();
                        goto Rbrack3;
                    }
                    lexer::getToken();
                Rbrack3:
                    if (lexer::getTokenType() == LBRACK)
                    {
                        lexer::getToken();
                        InitVarArray(2);
                        if (lexer::getTokenType() == RBRACK)
                        {
                            lexer::getToken();
                        }
                        else
                        {
                            parseError();
                        }
                    }
                }
            }
        }
    }
    outputFile << "<变量定义无初始化>" << endl;
}

static int dimension = 0;
static int length[2] = { -1, -1 };

#define CheckArray(dimension) do { \
if(length[dimension - 1] != varEntry->getLength(dimension)) { \
    errList.emplace_back(n, lexer::curToken);                    \
}                                  \
}while (false)

#define CheckConstType do { \
if (constType != varType){  \
   errList.emplace_back(o, lexer::curToken);                         \
}                            \
} while (false)

static void varDefWithInit()
{
    assert(isVarDef() == VARDEFWITHINIT);
    BaseType varType = typeSpec();
    BaseType constType;
    SymTabEntry* varEntry;
    VarBase* initVal;
    if (lexer::getTokenType() == IDENFR)
    { // Dimension 0 Left
        SymTabAddVar;
        VarBase* var = new Var(varEntry);
        lexer::getToken();
#ifdef ARRAYSUPPORT
        if (lexer::getTokenType() == LBRACK)
        {
            lexer::getToken();
            InitVarArray(1);
            if (lexer::getTokenType() != RBRACK)
            {
                RbrackExpected;
                parseError();
                goto Rbrack1;
            }
            else
            {// Dimension 1 Left
                lexer::getToken();
            Rbrack1:
                if (lexer::getTokenType() == LBRACK)
                {
                    lexer::getToken();
                    InitVarArray(2);
                    if (lexer::getTokenType() != RBRACK)
                    {
                        RbrackExpected;
                        parseError();
                        goto Rbrack2;
                    }
                    else
                    { // Dimension 2 Left
                        lexer::getToken();
                    Rbrack2:
                        if (lexer::getTokenType() == ASSIGN)
                        { // Dimension 2 Right Begin
                            dimension = 2;
                            assert(dimension == varEntry->getDimension());
                            lexer::getToken();
                            if (lexer::getTokenType() == LBRACE)
                            { // Outer Brace
                                // Dimension 1 Init
                                length[0] = 0;
                                lexer::getToken();
                                if (lexer::getTokenType() == LBRACE)
                                { // Inner Brace
                                    // Dimension 2 Init
                                    length[1] = 0;
                                    lexer::getToken();
                                    constType = constExpr(initVal);
                                    midCodes.push_back(new AssignMid(ADD_OP, initVal,
                                        new ArrayVar(varEntry,
                                            new ConstVar(length[0]),
                                            new ConstVar(length[1]))));
                                    CheckConstType;
                                    length[1]++;
                                    while (lexer::getTokenType() == COMMA)
                                    {
                                        lexer::getToken();
                                        constType = constExpr(initVal);
                                        midCodes.push_back(new AssignMid(ADD_OP, initVal,
                                            new ArrayVar(varEntry,
                                                new ConstVar(length[0]),
                                                new ConstVar(length[1]))));
                                        CheckConstType;
                                        length[1]++;
                                    }
                                    if (lexer::getTokenType() == RBRACE)
                                    {
                                        length[0]++;
                                        // Dimension 2 Check
                                        CheckArray(2);
                                        lexer::getToken();
                                        while (lexer::getTokenType() == COMMA)
                                        {
                                            lexer::getToken();
                                            if (lexer::getTokenType() == LBRACE)
                                            { // Inner Brace
                                                // Dimension 2 Init
                                                length[1] = 0;
                                                lexer::getToken();
                                                constType = constExpr(initVal);
                                                midCodes.push_back(new AssignMid(ADD_OP, initVal,
                                                    new ArrayVar(varEntry,
                                                        new ConstVar(length[0]),
                                                        new ConstVar(length[1]))));
                                                CheckConstType;
                                                length[1]++;
                                                while (lexer::getTokenType() == COMMA)
                                                {
                                                    lexer::getToken();
                                                    constType = constExpr(initVal);
                                                    midCodes.push_back(new AssignMid(ADD_OP, initVal,
                                                        new ArrayVar(varEntry,
                                                            new ConstVar(length[0]),
                                                            new ConstVar(length[1]))));
                                                    CheckConstType;
                                                    length[1]++;
                                                }
                                                if (lexer::getTokenType() == RBRACE)
                                                {
                                                    CheckArray(2);
                                                    lexer::getToken();
                                                }
                                                else
                                                {
                                                    parseError();
                                                }
                                            }
                                        }
                                        if (lexer::getTokenType() == RBRACE)
                                        {
                                            length[0]++;
                                            CheckArray(1);
                                            lexer::getToken();
                                        }
                                        else
                                        {
                                            parseError();
                                        }
                                    }
                                    else
                                    {
                                        parseError();
                                    }
                                }
                                else
                                {
                                    parseError();
                                }
                            }
                            else
                            {
                                parseError();
                            }
                            // Dimension 2 Right End
                        }
                    }
                }
                else if (lexer::getTokenType() == ASSIGN)
                { // Dimension 1 Right
                    dimension = 1;
                    assert(dimension == varEntry->getDimension());
                    lexer::getToken();
                    if (lexer::getTokenType() == LBRACE)
                    {
                        // Dimension 1 Init
                        length[0] = 0;
                        lexer::getToken();
                        constType = constExpr(initVal);
                        midCodes.push_back(new AssignMid(ADD_OP, initVal,
                            new ArrayVar(varEntry,
                                new ConstVar(length[0]))));
                        CheckConstType;
                        length[0]++;
                        while (lexer::getTokenType() == COMMA)
                        {
                            lexer::getToken();
                            constType = constExpr(initVal);
                            midCodes.push_back(new AssignMid(ADD_OP, initVal,
                                new ArrayVar(varEntry,
                                    new ConstVar(length[0]))));
                            CheckConstType;
                            length[0]++;
                        }
                        if (lexer::getTokenType() == RBRACE)
                        {
                            CheckArray(1);
                            lexer::getToken();
                        }
                    }

                }
            }
        }
        else
#endif
            if (lexer::getTokenType() == ASSIGN)
            { // Dimension 0 Right
                lexer::getToken();
                constType = constExpr(initVal);
                CheckConstType;
                varEntry->setInitVal(((ConstVar*)initVal)->value);
                if (curLayer != 0)
                {
                    midCodes.push_back(new AssignMid(ADD_OP, initVal, var));
                }
            }
            else
            {
                parseError();
            }
    }
    outputFile << "<变量定义及初始化>" << endl;
}

static SymTabEntry* defHead()
{
    TokenType retType = lexer::getTokenType();
    if (retType == INTTK || retType == CHARTK)
    {
        lexer::getToken();
        if (lexer::getTokenType() == IDENFR)
        {
            const string& funcName = lexer::curToken->getStr();
            const BaseType baseType = retType == INTTK ? INT : CHAR;
            lexer::getToken();
            outputFile << "<声明头部>" << endl;
            return new SymTabEntry(funcName, baseType, lexer::prevToken->getLine());
        }
    }
    else
    {
        parseError();
    }
    return nullptr;
}

static void paramTable()
{
    BaseType paramType;
    string paramName;
    if (lexer::getTokenType() == RPARENT
        || lexer::getTokenType() == SEMICN)
    {
        outputFile << "<参数表>" << endl;
        return;
    }
    else if (lexer::getTokenType() == INTTK || lexer::getTokenType() == CHARTK)
    {
        paramType = typeSpec();
        if (lexer::getTokenType() == IDENFR)
        {
            paramName = lexer::curToken->getStr();
            curParamTab->addParam(paramType, paramName);
            SymTabs[curLayer]->addEntry(paramName,
                new SymTabEntry(paramName, paramType,
                    lexer::curToken->getLine()));
            lexer::getToken();
            while (lexer::getTokenType() == COMMA)
            {
                lexer::getToken();
                paramType = typeSpec();
                if (lexer::getTokenType() == IDENFR)
                {
                    paramName = lexer::curToken->getStr();
                    curParamTab->addParam(paramType, paramName);
                    SymTabs[curLayer]->addEntry(paramName,
                        new SymTabEntry(paramName, paramType,
                            lexer::curToken->getLine()));
                    lexer::getToken();
                }
                else
                {
                    parseError();
                }
            }
            outputFile << "<参数表>" << endl;
            return;
        }
        else
        {
            parseError();
        }
    }
    else
    {
        parseError();
    }
}

static void compoundStat()
{
    if (lexer::getTokenType() == CONSTTK)
    {
        constDec();
    }
    if (isVarDec())
    {
        varDec();
    }
    stats();
    outputFile << "<复合语句>" << endl;
}

// Also getCurType
static BaseType typeSpec()
{
    switch (lexer::getTokenType())
    {
    case INTTK:
        lexer::getToken();
        return INT;
    case CHARTK:
        lexer::getToken();
        return CHAR;
    default:
        parseError();
        return VOID;
    }
}

#define CheckUndefinedIdentifier do { \
if (!SymTabs[curLayer]->exist(lexer::curToken->getStr())) \
{                                     \
errList.emplace_back(c, lexer::curToken);                 \
SymTabEntry* undefinedEntry =  new SymTabEntry(lexer::curToken->getStr(), UNDEF, \
lexer::curToken->getLine());                                     \
SymTabs[curLayer]->addEntry(lexer::curToken->getStr(), undefinedEntry); }        \
} while (false)

#define CheckConstAssign do { \
SymTabEntry* assignEntry = SymTabs[curLayer]->find(lexer::curToken->getStr()); \
if(assignEntry->isConst()){  \
    errList.emplace_back(j, lexer::curToken->getLine());                              \
}                             \
} while(false)

static void stats()
{
    while (isStat() != NONSTAT)
    {
        stat();
    }
    outputFile << "<语句列>" << endl;
}

static StatType isStat()
{
    Token* next;
    TokenType nextType;
    BaseType retType;
    string id;
    SymTabEntry* ptr;
    switch (lexer::getTokenType())
    {
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
        if (nextType == ASSIGN
            || nextType == LBRACK)
        {
            return ASSIGNSTAT;
        }
        else if (nextType == LPARENT)
        {
            ptr = getGSymTab()->find(id);
            if (ptr->isFunc())
            {
                retType = ptr->getBaseType();
                if (retType == VOID)
                {
                    return NONRETURNCALLSTAT;
                }
                else if (retType == INT || retType == CHAR)
                {
                    return RETURNCALLSTAT;
                }
            }
            else
            {
                parseError();
            }
        }
        else
        {
            parseError();
        }
    default:
        return NONSTAT;
    }
}

static void stat()
{
    switch (isStat())
    {
    case LOOPSTAT:
        loopStat();
        break;
    case IFSTAT:
        ifStat();
        break;
    case RETURNCALLSTAT:
        returnCallStat();
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
        }
        break;
    case NONRETURNCALLSTAT:
        nonReturnCallStat();
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
        }
        break;
    case ASSIGNSTAT:
        assignStat();
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
        }
        break;
    case READSTAT:
        readStat();
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
        }
        break;
    case WRITESTAT:
        writeStat();
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
        }
        break;
    case CASESTAT:
        caseStat();
        break;
    case RETURNSTAT:
        returnStat();
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
        }
        break;
    case VOIDSTAT:
        if (lexer::getTokenType() == SEMICN)
        {
            lexer::getToken();
        }
        else
        {
            SemicnExpected;
        }
        break;
    case STATS:
        if (lexer::getTokenType() == LBRACE)
        {
            lexer::getToken();
            stats();
            if (lexer::getTokenType() == RBRACE)
            {
                lexer::getToken();
            }
            else
            {
                parseError();
            }
        }
        else
        {
            parseError();
        }
        break;
    default:
        parseError();
        break;
    }
    outputFile << "<语句>" << endl;
}

static void loopStat()
{
    LoopLabel loopLabel;
    if (lexer::getTokenType() == WHILETK)
    {
        midCodes.push_back(new LabelMid(loopLabel.beginLabel));
        lexer::getToken();
        ACCEPT(LPARENT);
        lexer::getToken();
        VarBase* leftVar, * rightVar;
        MidOp op;
        condition(leftVar, op, rightVar);
        if (lexer::getTokenType() != RPARENT)
        {
            RparentExpected;
            goto Rparent1;
        }
        lexer::getToken();
    Rparent1:
        midCodes.push_back(new BranchMid(op, leftVar, rightVar, loopLabel.endLabel));
        stat();
        midCodes.push_back(new JumpMid(loopLabel.beginLabel));
        midCodes.push_back(new LabelMid(loopLabel.endLabel));
        outputFile << "<循环语句>" << endl;
        return;

    }
    else if (lexer::getTokenType() == FORTK)
    {
        lexer::getToken();
        ACCEPT(LPARENT);
        if (lexer::getTokenType() != IDENFR)
        {
            parseError();
            return;
        }
        CheckUndefinedIdentifier;
        SymTabEntry* loopVarEntry = SymTabs[curLayer]->find(lexer::curToken->getStr());
        Var* loopVar = new Var(loopVarEntry);
        lexer::getToken();
        ACCEPT(ASSIGN);
        VarBase* loopInit;
        expr(loopInit);
        if (lexer::getTokenType() != SEMICN)
        {
            SemicnExpected;
            parseError();
            goto semicn1;
        }
        lexer::getToken();
    semicn1:
        midCodes.push_back(new AssignMid(ADD_OP, loopInit, loopVar));
        VarBase* leftVar, * rightVar;
        MidOp op;
        condition(leftVar, op, rightVar);
        if (lexer::getTokenType() != SEMICN)
        {
            SemicnExpected;
            parseError();
            goto semicn2;
        }
        lexer::getToken();
    semicn2:
        midCodes.push_back(new LabelMid(loopLabel.beginLabel));
        midCodes.push_back(new BranchMid(op, leftVar, rightVar, loopLabel.endLabel));
        if (lexer::getTokenType() != IDENFR)
        {
            parseError();
            return;
        }
        CheckUndefinedIdentifier;
        SymTabEntry* updateEntryLeft = SymTabs[curLayer]->find(lexer::curToken->getStr());
        lexer::getToken();
        ACCEPT(ASSIGN);
        if (lexer::getTokenType() != IDENFR)
        {
            parseError();
            return;
        }
        CheckUndefinedIdentifier;
        SymTabEntry* updateEntryRight = SymTabs[curLayer]->find(lexer::curToken->getStr());
        lexer::getToken();
        if (lexer::getTokenType() == PLUS ||
            lexer::getTokenType() == MINU)
        {
            MidOp updateOp = MidOpMap[lexer::getTokenType()];
            lexer::getToken();
            int num = step();
            if (lexer::getTokenType() != RPARENT)
            {
                RparentExpected;
                goto Rparent2;
            }
            lexer::getToken();
        Rparent2:
            stat();
            midCodes.push_back(new AssignMid(updateOp,
                new Var(updateEntryRight), new ConstVar(num),
                new Var(updateEntryLeft)));
            midCodes.push_back(new JumpMid(loopLabel.beginLabel));
            midCodes.push_back(new LabelMid(loopLabel.endLabel));
            outputFile << "<循环语句>" << endl;
            return;
        }
    }
    else if (lexer::getTokenType() == DOTK)
    {
        midCodes.push_back(new LabelMid(loopLabel.beginLabel));
        lexer::getToken();
        stat();
        ACCEPT(WHILETK);
        ACCEPT(LPARENT);
        lexer::getToken();
        VarBase* leftVar, * rightVar;
        MidOp op;
        condition(leftVar, op, rightVar);
        if (lexer::getTokenType() != RPARENT)
        {
            RparentExpected;
            goto Rparent3;
        }
        lexer::getToken();
    Rparent3:
        midCodes.push_back(new BranchMid(op, leftVar, rightVar, loopLabel.beginLabel, true));
        midCodes.push_back(new LabelMid(loopLabel.endLabel));
        outputFile << "<循环语句>" << endl;
        return;
    }
    parseError();
}

static void ifStat()
{
    ACCEPT(IFTK);
    ACCEPT(LPARENT);
    IfLabel ifLabel;
    VarBase* leftVar, * rightVar;
    MidOp op;
    condition(leftVar, op, rightVar);
    midCodes.push_back(new BranchMid(op, leftVar, rightVar, ifLabel.elseLabel));
    if (lexer::getTokenType() != RPARENT)
    {
        RparentExpected;
        goto RParent;
    }
    lexer::getToken();
RParent:
    stat();
    midCodes.push_back(new JumpMid(ifLabel.endLabel));
    midCodes.push_back(new LabelMid(ifLabel.elseLabel));
    if (lexer::getTokenType() == ELSETK)
    {
        lexer::getToken();
        stat();
    }
    midCodes.push_back(new LabelMid(ifLabel.endLabel));
    outputFile << "<条件语句>" << endl;
    return;
    parseError();
}


#define GetCurFuncParamTab do { \
curFuncParamTab = SymTabs[curLayer]->find(lexer::curToken->getStr())->getParamTab(); \
} while (false)

#define GetCurBaseType(baseType) do { \
baseType = SymTabs[curLayer]->find(lexer::curToken->getStr())->getBaseType(); \
} while (false)

static BaseType returnCallStat()
{
    ParamTab* curFuncParamTab;
    SymTabEntry* curFuncSymTabEntry;
    CallMid* returnCall;
    if (lexer::getTokenType() == IDENFR)
    {
        curFuncSymTabEntry = SymTabs[curLayer]->find(lexer::curToken->getStr());
        GetCurFuncParamTab;
        BaseType ret;
        GetCurBaseType(ret);
        lexer::getToken();
        ACCEPT(LPARENT);
        returnCall = new CallMid(curFuncSymTabEntry);
        valueTable(curFuncParamTab, returnCall);
        if (lexer::getTokenType() != RPARENT)
        {
            RparentExpected;
            goto Rparent;
        }
        lexer::getToken();
    Rparent:
        outputFile << "<有返回值函数调用语句>" << endl;
        return ret;
    }
    parseError();
    return UNDEF;
}

static void nonReturnCallStat()
{
    ParamTab* curFuncParamTab;
    SymTabEntry* curFuncSymTabEntry;
    CallMid* nonReturnCall;
    if (lexer::getTokenType() == IDENFR)
    {
        curFuncSymTabEntry = SymTabs[curLayer]->find(lexer::curToken->getStr());
        GetCurFuncParamTab;
        lexer::getToken();
        ACCEPT(LPARENT);
        nonReturnCall = new CallMid(curFuncSymTabEntry);
        valueTable(curFuncParamTab, nonReturnCall);
        if (lexer::getTokenType() != RPARENT)
        {
            RparentExpected;
            goto Rparent;
        }
        else
        {
            lexer::getToken();
        Rparent:
            outputFile << "<无返回值函数调用语句>" << endl;
            return;
        }
    }
    parseError();
}

static void assignStat()
{
    BaseType sub;
    VarBase* subscript, * subscript1;
    VarBase* assignVal;
    if (lexer::getTokenType() == IDENFR)
    { // Dimension 0 Left
        CheckUndefinedIdentifier;
        CheckConstAssign;
        SymTabEntry* entry = SymTabs[curLayer]->find(lexer::curToken->getStr());
        Var* var = new Var(entry);
        lexer::getToken();
#ifdef ARRAYSUPPORT
        if (lexer::getTokenType() == LBRACK)
        {
            lexer::getToken();
            sub = expr(subscript);
            if (sub != INT)
            {
                errList.emplace_back(i, lexer::curToken);
            }
            if (lexer::getTokenType() != RBRACK)
            {
                RbrackExpected;
                goto Rbrack1;
            }
            else
            {// Dimension 1 Left
                lexer::getToken();
            Rbrack1:
                if (lexer::getTokenType() == LBRACK)
                {
                    lexer::getToken();
                    sub = expr(subscript1);
                    if (sub != INT)
                    {
                        errList.emplace_back(i, lexer::curToken);
                    }
                    if (lexer::getTokenType() != RBRACK)
                    {
                        RbrackExpected;
                        goto Rbrack2;
                    }
                    else
                    { // Dimension 2 Left
                        lexer::getToken();
                    Rbrack2:
                        if (lexer::getTokenType() == ASSIGN)
                        { // Dimension 2 Right
                            lexer::getToken();
                            expr(assignVal);
                            midCodes.push_back(new AssignMid(ADD_OP, assignVal,
                                new ArrayVar(entry, subscript, subscript1)));
                            outputFile << "<赋值语句>" << endl;
                            return;
                        }
                    }
                }
                else if (lexer::getTokenType() == ASSIGN)
                { // Dimension 1 Right
                    lexer::getToken();
                    expr(assignVal);
                    midCodes.push_back(new AssignMid(ADD_OP, assignVal,
                        new ArrayVar(entry, subscript)));
                    outputFile << "<赋值语句>" << endl;
                    return;
                }
            }
        }
        else
#endif
            if (lexer::getTokenType() == ASSIGN)
            { // Dimension 0 Right
                lexer::getToken();
                expr(assignVal);
                midCodes.push_back(new AssignMid(ADD_OP, assignVal, var));
                outputFile << "<赋值语句>" << endl;
                return;
            }
    }
    parseError();
}

static void readStat()
{
    ACCEPT(SCANFTK);
    ACCEPT(LPARENT);
    if (lexer::getTokenType() == IDENFR)
    {
        CheckUndefinedIdentifier;
        CheckConstAssign;
        SymTabEntry* entry = SymTabs[curLayer]->find(lexer::curToken->getStr());
        Var* var = new Var(entry);
        lexer::getToken();
        if (lexer::getTokenType() != RPARENT)
        {
            RparentExpected;
            goto Rparent;
        }
        lexer::getToken();
    Rparent:
        midCodes.push_back(new ReadMid(var));
        outputFile << "<读语句>" << endl;
        return;
    }
    parseError();
}

static void writeStat()
{
    ACCEPT(PRINTFTK);
    ACCEPT(LPARENT);
    if (lexer::getTokenType() == STRCON)
    {
        auto* stringVar = new StringVar(stringCon());
        midCodes.push_back(new WriteMid(stringVar));
        if (lexer::getTokenType() == COMMA)
        {
            lexer::getToken();
        }
        else if (lexer::getTokenType() == RPARENT)
        {
            lexer::getToken();
            midCodes.push_back(new WriteMid());
            outputFile << "<写语句>" << endl;
            return;
        }
        else
        {
            RparentExpected;
            midCodes.push_back(new WriteMid());
            outputFile << "<写语句>" << endl;
            return;
        }
    }
    VarBase* exprVar;
    BaseType exprType = expr(exprVar);
    if (lexer::getTokenType() == RPARENT)
    {
        lexer::getToken();
        midCodes.push_back(new WriteMid(exprVar, exprType == CHAR));
        midCodes.push_back(new WriteMid());
        outputFile << "<写语句>" << endl;
        return;
    }
    else
    {
        RparentExpected;
        midCodes.push_back(new WriteMid(exprVar, exprType == CHAR));
        midCodes.push_back(new WriteMid());
        outputFile << "<写语句>" << endl;
        return;
    }
    parseError();
}

#define CheckCaseType do { \
if (constType != caseType){  \
   errList.emplace_back(o, lexer::curToken);                         \
}                            \
} while (false)

static void caseStat()
{
    BaseType caseType;
    ACCEPT(SWITCHTK);
    ACCEPT(LPARENT);
    VarBase* caseVar;
    caseType = expr(caseVar);
    if (lexer::getTokenType() != RPARENT)
    {
        RparentExpected;
        goto Rparent;
    }
    lexer::getToken();
Rparent:
    SwitchLabel switchLabel(caseVar);
    ACCEPT(LBRACE);
    caseTable(caseType, switchLabel);
    if (lexer::getTokenType() == DEFAULTTK)
    {
        defaultStat(switchLabel);
    }
    else
    {
        errList.emplace_back(p, lexer::curToken);
    }
    ACCEPT(RBRACE);
    outputFile << "<情况语句>" << endl;
    return;
    parseError();
}

static void returnStat()
{
    VarBase* retVar = nullptr;
    BaseType ret = UNDEF;
    ACCEPT(RETURNTK);
    if (lexer::getTokenType() == LPARENT)
    {
        if (isNonReturnFunc)
        {
            errList.emplace_back(g, lexer::curToken->getLine());
        }
        lexer::getToken();
        if (lexer::getTokenType() != RPARENT
            && lexer::getTokenType() != SEMICN)
        {
            ret = expr(retVar);
        }
        if (lexer::getTokenType() == RPARENT)
        {
            lexer::getToken();
        Rparent:
            outputFile << "<返回语句>" << endl;
            if (isReturnFunc)
            {
                returnCount++;
                if (ret != returnType)
                {
                    errList.emplace_back(h, lexer::prevToken->getLine());
                }
            }
            midCodes.push_back(new ReturnMid(retVar));
            return;
        }
        else
        {
            RparentExpected;
            parseError();
            goto Rparent;
        }
    }
    outputFile << "<返回语句>" << endl;
    if (isReturnFunc)
    {
        returnCount++;
        errList.emplace_back(h, lexer::prevToken->getLine());
    }
    midCodes.push_back(new ReturnMid());
    return;
    parseError();
}

static void condition(VarBase*& leftVar, MidOp& op, VarBase*& rightVar)
{
    BaseType left, right;
    left = expr(leftVar);
    if (left != INT)
    {
        errList.emplace_back(f, lexer::prevToken->getLine());
    }
    op = relationOp();
    right = expr(rightVar);
    if (right != INT)
    {
        errList.emplace_back(f, lexer::prevToken->getLine());
    }
    outputFile << "<条件>" << endl;

}

static int step()
{
    int ret = unsignedInteger();
    outputFile << "<步长>" << endl;
    return ret;
}

static void valueTable(ParamTab* paramTab, CallMid* callMid)
{
    vector<BaseType> valueTable;
    if (lexer::getTokenType() == RPARENT ||
        lexer::getTokenType() == SEMICN)
    {
        outputFile << "<值参数表>" << endl;
        paramTab->checkParams(valueTable, lexer::curToken->getLine());
        return;
    }
    VarBase* curParam;
    valueTable.push_back(expr(curParam));
    callMid->valTable.push_back(curParam);
    while (lexer::getTokenType() == COMMA)
    {
        lexer::getToken();
        valueTable.push_back(expr(curParam));
        callMid->valTable.push_back(curParam);
    }
    outputFile << "<值参数表>" << endl;
    paramTab->checkParams(valueTable, lexer::curToken->getLine());
}

static void caseTable(BaseType caseType, SwitchLabel switchLabel)
{
    do
    {
        caseSubStat(caseType, switchLabel);
    } while (isCaseSubStat());
    outputFile << "<情况表>" << endl;
}

static bool isCaseSubStat()
{
    return lexer::getTokenType() == CASETK;
}

static void caseSubStat(BaseType caseType, SwitchLabel switchLabel)
{
    midCodes.push_back(new LabelMid(switchLabel.curCaseLabel()));
    BaseType constType;
    ACCEPT(CASETK);
    VarBase* caseSubVar;
    constType = constExpr(caseSubVar);
    CheckCaseType;
    ACCEPT(COLON);
    midCodes.push_back(new BranchMid(NEQ_OP,
        switchLabel.caseVar, caseSubVar,
        switchLabel.nextCaseLabel(), false));
    stat();
    outputFile << "<情况子语句>" << endl;
}

static void defaultStat(SwitchLabel switchLabel)
{
    midCodes.push_back(new LabelMid(switchLabel.curCaseLabel()));
    ACCEPT(DEFAULTTK);
    ACCEPT(COLON);
    stat();
    outputFile << "<缺省>" << endl;
}

static int integer()
{
    int ret = 1;
    if (lexer::getTokenType() == MINU)
    {
        ret = -1;
    }
    if (lexer::getTokenType() == PLUS ||
        lexer::getTokenType() == MINU)
    {
        lexer::getToken();
    }
    ret *= unsignedInteger();
    outputFile << "<整数>" << endl;
    return ret;
    parseError();
}

static int unsignedInteger()
{
    assert(lexer::getTokenType() == INTCON);
    int ret = stoi(lexer::curToken->getStr());
    ACCEPT(INTCON);
    outputFile << "<无符号整数>" << endl;
    return ret;
}

static char character()
{
    assert(lexer::getTokenType() == CHARCON);
    char ret = lexer::curToken->getStr().at(0);
    ACCEPT(CHARCON);
    return ret;
}

static string stringCon()
{
    assert(lexer::getTokenType() == STRCON);
    const string& ret = lexer::curToken->getStr();
    ACCEPT(STRCON);
    outputFile << "<字符串>" << endl;
    return ret;
}

static BaseType expr(VarBase*& var)
{
    VarBase* curVar;
    BaseType ret = UNDEF;
    TokenType firstType = lexer::getTokenType();
    bool isNegative = firstType == MINU;
    if (firstType == PLUS || firstType == MINU)
    {
        ret = INT;
        lexer::getToken();
    }
    BaseType termType = term(curVar);
    if (ret == UNDEF)
    {
        ret = termType;
    }
    if (isNegative)
    {
        VarBase* tempVar = new TempVar();
        midCodes.push_back(new AssignMid(SUB_OP, curVar, tempVar));
        curVar = tempVar;
    }
    while (lexer::getTokenType() == PLUS ||
        lexer::getTokenType() == MINU)
    {
        MidOp termOp = MidOpMap[lexer::getTokenType()];
        ret = INT;
        lexer::getToken();
        VarBase* termVar;
        term(termVar);
        VarBase* tempVar = new TempVar();
        midCodes.push_back(new AssignMid(termOp, curVar, termVar, tempVar));
        curVar = tempVar;
    }
    var = curVar;
    outputFile << "<表达式>" << endl;
    return ret;
}

static BaseType term(VarBase*& var)
{
    VarBase* curVar;
    BaseType ret = factor(curVar);
    while (lexer::getTokenType() == MULT ||
        lexer::getTokenType() == DIV)
    {
        MidOp factorOp = MidOpMap[lexer::getTokenType()];
        ret = INT;
        lexer::getToken();
        VarBase* factorVar;
        factor(factorVar);
        VarBase* tempVar = new TempVar();
        midCodes.push_back(new AssignMid(factorOp, curVar, factorVar, tempVar));
        curVar = tempVar;
    }
    var = curVar;
    outputFile << "<项>" << endl;
    return ret;
}

static BaseType factor(VarBase*& var)
{
    BaseType ret = INT;
    BaseType sub;
    VarBase* subscript, * subscript1;
#ifdef FUNCTIONSUPPORT
    if (isStat() == RETURNCALLSTAT)
    {
        ret = returnCallStat();
        var = new RetVar();
        outputFile << "<因子>" << endl;
        return ret;
    }
#endif
    if (lexer::getTokenType() == IDENFR)
    { // Dimension 0 Left
        CheckUndefinedIdentifier;
        GetCurBaseType(ret);
        SymTabEntry* entry = SymTabs[curLayer]->find(lexer::curToken->getStr());
        lexer::getToken();
#ifdef ARRAYSUPPORT
        if (lexer::getTokenType() == LBRACK)
        {
            lexer::getToken();
            sub = expr(subscript);
            if (sub != INT)
            {
                errList.emplace_back(i, lexer::curToken);
            }
            if (lexer::getTokenType() != RBRACK)
            {
                parseError();
                RbrackExpected;
                goto Rbrack1;
            }
            else
            {// Dimension 1 Left
                lexer::getToken();
            Rbrack1:
                if (lexer::getTokenType() == LBRACK)
                {
                    lexer::getToken();
                    sub = expr(subscript1);
                    if (sub != INT)
                    {
                        errList.emplace_back(i, lexer::curToken);
                    }
                    if (lexer::getTokenType() == RBRACK)
                    { // Dimension 2 Left
                        lexer::getToken();
                    Rbrack2:
                        var = new ArrayVar(entry, subscript, subscript1);
                        outputFile << "<因子>" << endl;
                        return ret;
                    }
                    else
                    {
                        parseError();
                        RbrackExpected;
                        goto Rbrack2;
                    }
                }
                var = new ArrayVar(entry, subscript);
                outputFile << "<因子>" << endl;
                return ret;
            }
        }
#endif
        if (entry->isConst())
        {
            var = new ConstVar(entry->getInitVal());
        }
        else
        {
            var = new Var(entry);
        }
        // TODO : Replace const var?
        outputFile << "<因子>" << endl;
        return ret;
    }
    else if (lexer::getTokenType() == PLUS ||
        lexer::getTokenType() == MINU ||
        lexer::getTokenType() == INTCON)
    {
        var = new ConstVar(integer());
        outputFile << "<因子>" << endl;
        return ret;
    }
    else if (lexer::getTokenType() == CHARCON)
    {
        ret = CHAR;
        var = new ConstVar(character());
        outputFile << "<因子>" << endl;
        return ret;
    }
    else if (lexer::getTokenType() == LPARENT)
    {
        lexer::getToken();
        expr(var);
        if (lexer::getTokenType() == RPARENT)
        {
            lexer::getToken();
            outputFile << "<因子>" << endl;
            return ret;
        }
        else
        {
            RparentExpected;
            outputFile << "<因子>" << endl;
            return ret;
        }
    }
    return UNDEF;
}

static MidOp relationOp()
{
    TokenType type = lexer::getTokenType();
    switch (type)
    {
    case LSS:
    case LEQ:
    case GRE:
    case GEQ:
    case EQL:
    case NEQ:
        lexer::getToken();
        return MidOpMap[type];
    default:
        parseError();
    }
    return ILLEGAL_OP;
}

static BaseType constExpr(VarBase*& var)
{
    switch (lexer::getTokenType())
    {
    case INTCON:
    case PLUS:
    case MINU:
        var = new ConstVar(integer());
        outputFile << "<常量>" << endl;
        return INT;
    case CHARCON:
        var = new ConstVar(character());
        outputFile << "<常量>" << endl;
        return CHAR;
    default:
        break;
    }
    parseError();
    return UNDEF;
}