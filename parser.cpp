#include "lexer.h"
#include <cassert>
#include "SymbolTable.h"
#include "Error.h"
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
static int integer();// Native: +|- unsignedInteger()
static int unsignedInteger();// Native: <INTCON>
static char character(); // Native: <CHARCON>
static void stringCon();

static void expr(); // Native: +|-, term()
static void term(); // Native: factor(), * /
static void factor(); // Native: <IDENFR>, [], integer(), character(), returnCallStat()
static void relationOp(); // Native: < | <= | > | >= | != | ==
static void constExpr();

// error
#define parseError() do { \
    cout << "Parser Error! At " << __FUNCTION__ << ":" << __LINE__ << endl; \
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

static void returnFuncDef()
{
	bool override = false;
	SymTabs[++curLayer] = new SymTab();
	auto* newEntry = defHead();
	SymTabAddFunc;
	if (lexer::getTokenType() == LPARENT)
	{
		lexer::getToken();
		if (!override)
		{ curParamTab = new ParamTab(); }
		paramTable();
		newEntry->setParamTab(curParamTab);
		if (lexer::getTokenType() == RPARENT)
		{
			lexer::getToken();
			if (lexer::getTokenType() == LBRACE)
			{
				lexer::getToken();
				compoundStat();
				if (lexer::getTokenType() == RBRACE)
				{
					lexer::getToken();
					outputFile << "<有返回值函数定义>" << endl;
					newEntry->setSymTab(SymTabs[curLayer--]);
					return;
				}
			}
		}
	}
	parseError();
}

static void nonReturnFuncDef()
{
	bool override = false;
	assert(lexer::getTokenType() == VOIDTK);
	SymTabs[++curLayer] = new SymTab();
	lexer::getToken();
	if (lexer::getTokenType() == IDENFR)
	{
		const string& funcName = lexer::curToken->getStr();
		auto* newEntry = new SymTabEntry(funcName, VOID, lexer::curToken->getLine());
		SymTabAddFunc;
		lexer::getToken();
		if (lexer::getTokenType() == LPARENT)
		{
			lexer::getToken();
			if (!override)
			{ curParamTab = new ParamTab(); }
			paramTable();
			newEntry->setParamTab(curParamTab);
			if (lexer::getTokenType() == RPARENT)
			{
				lexer::getToken();
				if (lexer::getTokenType() == LBRACE)
				{
					lexer::getToken();
					compoundStat();
					if (lexer::getTokenType() == RBRACE)
					{
						lexer::getToken();
						outputFile << "<无返回值函数定义>" << endl;
						newEntry->setSymTab(SymTabs[curLayer--]);
						return;
					}
				}
			}
		}
	}
	parseError();
}

static void mainFunc()
{
	assert(curLayer == 0);
	SymTabEntry* mainEntry;
	if (lexer::getTokenType() == VOIDTK)
	{
		lexer::getToken();
		if (lexer::getTokenType() == MAINTK)
		{
			// Add SymTab of void main()
			mainEntry = new
					SymTabEntry("main", VOID,
					lexer::curToken->getLine());
			getGSymTab()->addEntry("main", mainEntry);
			SymTabs[++curLayer] = new SymTab();
			lexer::getToken();
			if (lexer::getTokenType() == LPARENT)
			{
				lexer::getToken();
				if (lexer::getTokenType() == RPARENT)
				{
					lexer::getToken();
					if (lexer::getTokenType() == LBRACE)
					{
						lexer::getToken();
						compoundStat();
						if (lexer::getTokenType() == RBRACE)
						{
							lexer::getToken();
							outputFile << "<主函数>" << endl;
							mainEntry->setSymTab(SymTabs[curLayer--]);
							return;
						}
					}
				}
			}
		}
	}
}

#define SymTabAddConst do { \
    constEntry = new \
        SymTabEntry(lexer::curToken->getStr(), constType, \
            lexer::curToken->getLine());                  \
    constEntry->setConst(true);                           \
    SymTabs[curLayer]->addEntry(lexer::curToken->getStr(), \
        constEntry); } while (false)

static void constDef()
{
	BaseType constType = typeSpec();
	SymTabEntry* constEntry;
	if (lexer::getTokenType() == INTTK)
	{
		lexer::getToken();
		if (lexer::getTokenType() == IDENFR)
		{
			SymTabAddConst;
			lexer::getToken();
			if (lexer::getTokenType() == ASSIGN)
			{
				lexer::getToken();
				constEntry->setInitVal(integer());
				while (lexer::getTokenType() == COMMA)
				{
					lexer::getToken();
					if (lexer::getTokenType() == IDENFR)
					{
						SymTabAddConst;
						lexer::getToken();
						if (lexer::getTokenType() == ASSIGN)
						{
							lexer::getToken();
							constEntry->setInitVal(integer());
						}
					}
				}
				outputFile << "<常量定义>" << endl;
				return;
			}
		}
	}
	else if (lexer::getTokenType() == CHARTK)
	{
		lexer::getToken();
		if (lexer::getTokenType() == IDENFR)
		{
			SymTabAddConst;
			lexer::getToken();
			if (lexer::getTokenType() == ASSIGN)
			{
				lexer::getToken();
				character();
				while (lexer::getTokenType() == COMMA)
				{
					lexer::getToken();
					if (lexer::getTokenType() == IDENFR)
					{
						SymTabAddConst;
						lexer::getToken();
						if (lexer::getTokenType() == ASSIGN)
						{
							lexer::getToken();
							constEntry->setInitVal(character());
						}
					}
				}
				outputFile << "<常量定义>" << endl;
				return;
			}
		}
	}
	parseError();
}

static VarDefType isVarDef()
{
	if (lexer::getTokenType() == INTTK || lexer::getTokenType() == CHARTK)
	{
		Token* next2 = lexer::preFetch(2), * next5, * next8;
		TokenType nextType2 = next2->getType(), nextType5, nextType8;
		switch (nextType2)
		{
		case SEMICN:
		case COMMA:
			return VARDEFNOINIT;
		case ASSIGN:
			return VARDEFWITHINIT;
		case LBRACK:
			next5 = lexer::preFetch(5);
			nextType5 = next5->getType();
			switch (nextType5)
			{
			case SEMICN:
			case COMMA:
				return VARDEFNOINIT;
			case ASSIGN:
				return VARDEFWITHINIT;
			case LBRACK:
				next8 = lexer::preFetch(8);
				nextType8 = next8->getType();
				switch (nextType8)
				{
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
            lexer::curToken->getLine()); \
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
			if (lexer::getTokenType() == RBRACK)
			{
				lexer::getToken();
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
			else
			{
				parseError();
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
					if (lexer::getTokenType() == RBRACK)
					{
						lexer::getToken();
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
					else
					{
						parseError();
					}
				}
			}
		}
	}
	outputFile << "<变量定义无初始化>" << endl;
}


// TODO: Initialization of var(s)
static void varDefWithInit()
{
	assert(isVarDef() == VARDEFWITHINIT);
	BaseType varType = typeSpec();
	SymTabEntry* varEntry;
	if (lexer::getTokenType() == IDENFR)
	{ // Dimension 0 Left
		SymTabAddVar;
		lexer::getToken();
		if (lexer::getTokenType() == LBRACK)
		{
			lexer::getToken();
			InitVarArray(1);
			if (lexer::getTokenType() == RBRACK)
			{// Dimension 1 Left
				lexer::getToken();
				if (lexer::getTokenType() == LBRACK)
				{
					lexer::getToken();
					InitVarArray(2);
					if (lexer::getTokenType() == RBRACK)
					{ // Dimension 2 Left
						lexer::getToken();
						if (lexer::getTokenType() == ASSIGN)
						{ // Dimension 2 Right Begin
							lexer::getToken();
							if (lexer::getTokenType() == LBRACE)
							{ // Outer Brace
								lexer::getToken();
								if (lexer::getTokenType() == LBRACE)
								{ // Inner Brace
									lexer::getToken();
									constExpr();
									while (lexer::getTokenType() == COMMA)
									{
										lexer::getToken();
										constExpr();
									}
									if (lexer::getTokenType() == RBRACE)
									{
										lexer::getToken();
										while (lexer::getTokenType() == COMMA)
										{
											lexer::getToken();
											if (lexer::getTokenType() == LBRACE)
											{ // Inner Brace
												lexer::getToken();
												constExpr();
												while (lexer::getTokenType() == COMMA)
												{
													lexer::getToken();
													constExpr();
												}
												if (lexer::getTokenType() == RBRACE)
												{
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
								//while (lexer::getTokenType() == COMMA) {
								//    lexer::getToken();
								//    if (lexer::getTokenType() == LBRACE) { // Inner Brace
								//        lexer::getToken();
								//        constExpr();
								//        while (lexer::getTokenType() == COMMA) {
								//            lexer::getToken();
								//            constExpr();
								//        }
								//        if (lexer::getTokenType() == RBRACE) {
								//            lexer::getToken();
								//        }
								//        else {
								//            error();
								//        }
								//    }
								//    else {
								//        error();
								//    }
								//}
							}
							else
							{
								parseError();
							}
							// Dimension 2 Right End
						}
					}
					else
					{
						parseError();
					}
				}
				else if (lexer::getTokenType() == ASSIGN)
				{ // Dimension 1 Right
					lexer::getToken();
					if (lexer::getTokenType() == LBRACE)
					{
						lexer::getToken();
						constExpr();
						while (lexer::getTokenType() == COMMA)
						{
							lexer::getToken();
							constExpr();
						}
						lexer::getToken();
						if (lexer::getTokenType() == RBRACE)
						{
							lexer::getToken();
						}
					}

				}
			}
			else
			{
				parseError();
			}
		}
		else if (lexer::getTokenType() == ASSIGN)
		{ // Dimension 0 Right
			lexer::getToken();
			constExpr();
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
	if (lexer::getTokenType() == RPARENT)
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
		break;
	case NONRETURNCALLSTAT:
		nonReturnCallStat();
		if (lexer::getTokenType() == SEMICN)
		{
			lexer::getToken();
		}
		break;
	case ASSIGNSTAT:
		assignStat();
		if (lexer::getTokenType() == SEMICN)
		{
			lexer::getToken();
		}
		break;
	case READSTAT:
		readStat();
		if (lexer::getTokenType() == SEMICN)
		{
			lexer::getToken();
		}
		break;
	case WRITESTAT:
		writeStat();
		if (lexer::getTokenType() == SEMICN)
		{
			lexer::getToken();
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
		break;
	case VOIDSTAT:
		if (lexer::getTokenType() == SEMICN)
		{
			lexer::getToken();
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
	if (lexer::getTokenType() == WHILETK)
	{
		lexer::getToken();
		if (lexer::getTokenType() == LPARENT)
		{
			lexer::getToken();
			condition();
			if (lexer::getTokenType() == RPARENT)
			{
				lexer::getToken();
				stat();
				outputFile << "<循环语句>" << endl;
				return;
			}
		}
	}
	else if (lexer::getTokenType() == FORTK)
	{
		lexer::getToken();
		if (lexer::getTokenType() == LPARENT)
		{
			lexer::getToken();
			if (lexer::getTokenType() == IDENFR)
			{
				CheckUndefinedIdentifier;
				lexer::getToken();
				if (lexer::getTokenType() == ASSIGN)
				{
					lexer::getToken();
					expr();
					if (lexer::getTokenType() == SEMICN)
					{
						lexer::getToken();
						condition();
						if (lexer::getTokenType() == SEMICN)
						{
							lexer::getToken();
							if (lexer::getTokenType() == IDENFR)
							{
								CheckUndefinedIdentifier;
								lexer::getToken();
								if (lexer::getTokenType() == ASSIGN)
								{
									lexer::getToken();
									if (lexer::getTokenType() == IDENFR)
									{
										CheckUndefinedIdentifier;
										// TODO
										lexer::getToken();
										if (lexer::getTokenType() == PLUS ||
											lexer::getTokenType() == MINU)
										{
											lexer::getToken();
											step();
											if (lexer::getTokenType() == RPARENT)
											{
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
	parseError();
}

static void ifStat()
{
	if (lexer::getTokenType() == IFTK)
	{
		lexer::getToken();
		if (lexer::getTokenType() == LPARENT)
		{
			lexer::getToken();
			condition();
			if (lexer::getTokenType() == RPARENT)
			{
				lexer::getToken();
				stat();
				if (lexer::getTokenType() == ELSETK)
				{
					lexer::getToken();
					stat();
				}
				outputFile << "<条件语句>" << endl;
				return;
			}
		}
	}
	parseError();
}

static void returnCallStat()
{
	if (lexer::getTokenType() == IDENFR)
	{
		lexer::getToken();
		if (lexer::getTokenType() == LPARENT)
		{
			lexer::getToken();
			valueTable();
			if (lexer::getTokenType() == RPARENT)
			{
				lexer::getToken();
				outputFile << "<有返回值函数调用语句>" << endl;
				return;
			}
		}
	}
	parseError();
}

static void nonReturnCallStat()
{
	if (lexer::getTokenType() == IDENFR)
	{
		lexer::getToken();
		if (lexer::getTokenType() == LPARENT)
		{
			lexer::getToken();
			valueTable();
			if (lexer::getTokenType() == RPARENT)
			{
				lexer::getToken();
				outputFile << "<无返回值函数调用语句>" << endl;
				return;
			}
		}
	}
	parseError();
}

static void assignStat()
{
	if (lexer::getTokenType() == IDENFR)
	{ // Dimension 0 Left
		CheckUndefinedIdentifier;
			lexer::getToken();
			if (lexer::getTokenType() == LBRACK)
			{
				lexer::getToken();
				expr();
				if (lexer::getTokenType() == RBRACK)
				{// Dimension 1 Left
					lexer::getToken();
					if (lexer::getTokenType() == LBRACK)
					{
						lexer::getToken();
						expr();
						if (lexer::getTokenType() == RBRACK)
						{ // Dimension 2 Left
							lexer::getToken();
							if (lexer::getTokenType() == ASSIGN)
							{ // Dimension 2 Right
								lexer::getToken();
								expr();
								outputFile << "<赋值语句>" << endl;
								return;
							}
						}
					}
					else if (lexer::getTokenType() == ASSIGN)
					{ // Dimension 1 Right
						lexer::getToken();
						expr();
						outputFile << "<赋值语句>" << endl;
						return;
					}
				}
			}
			else if (lexer::getTokenType() == ASSIGN)
			{ // Dimension 0 Right
				lexer::getToken();
				expr();
				outputFile << "<赋值语句>" << endl;
				return;
			}
		}
		parseError();
	}

	static void readStat()
	{
		if (lexer::getTokenType() == SCANFTK)
		{
			lexer::getToken();
			if (lexer::getTokenType() == LPARENT)
			{
				lexer::getToken();
				if (lexer::getTokenType() == IDENFR)
				{
					CheckUndefinedIdentifier;
					lexer::getToken();
					if (lexer::getTokenType() == RPARENT)
					{
						lexer::getToken();
						outputFile << "<读语句>" << endl;
						return;
					}
				}
			}
		}
		parseError();
	}

	static void writeStat()
	{
		if (lexer::getTokenType() == PRINTFTK)
		{
			lexer::getToken();
			if (lexer::getTokenType() == LPARENT)
			{
				lexer::getToken();
				if (lexer::getTokenType() == STRCON)
				{
					stringCon();
					if (lexer::getTokenType() == COMMA)
					{
						lexer::getToken();
					}
					else if (lexer::getTokenType() == RPARENT)
					{
						lexer::getToken();
						outputFile << "<写语句>" << endl;
						return;
					}
				}
				expr();
				if (lexer::getTokenType() == RPARENT)
				{
					lexer::getToken();
					outputFile << "<写语句>" << endl;
					return;
				}
			}
		}
		parseError();
	}

	static void caseStat()
	{
		if (lexer::getTokenType() == SWITCHTK)
		{
			lexer::getToken();
			if (lexer::getTokenType() == LPARENT)
			{
				lexer::getToken();
				expr();
				if (lexer::getTokenType() == RPARENT)
				{
					lexer::getToken();
					if (lexer::getTokenType() == LBRACE)
					{
						lexer::getToken();
						caseTable();
						defaultStat();
						if (lexer::getTokenType() == RBRACE)
						{
							lexer::getToken();
							outputFile << "<情况语句>" << endl;
							return;
						}
					}
				}
			}
		}
		parseError();
	}

	static void returnStat()
	{
		if (lexer::getTokenType() == RETURNTK)
		{
			lexer::getToken();
			if (lexer::getTokenType() == LPARENT)
			{
				lexer::getToken();
				expr();
				if (lexer::getTokenType() == RPARENT)
				{
					lexer::getToken();
					outputFile << "<返回语句>" << endl;
					return;
				}
				else
				{
					parseError();
				}
			}
			outputFile << "<返回语句>" << endl;
			return;
		}
		parseError();
	}

	static void condition()
	{
		expr();
		relationOp();
		expr();
		outputFile << "<条件>" << endl;
	}

	static void step()
	{
		unsignedInteger();
		outputFile << "<步长>" << endl;
	}

	static void valueTable()
	{
		if (lexer::getTokenType() == RPARENT)
		{
			outputFile << "<值参数表>" << endl;
			return;
		}
		expr();
		while (lexer::getTokenType() == COMMA)
		{
			lexer::getToken();
			expr();
		}
		outputFile << "<值参数表>" << endl;
	}

	static void caseTable()
	{
		do
		{
			caseSubStat();
		} while (isCaseSubStat());
		outputFile << "<情况表>" << endl;
	}

	static bool isCaseSubStat()
	{
		return lexer::getTokenType() == CASETK;
	}

	static void caseSubStat()
	{
		if (isCaseSubStat())
		{
			lexer::getToken();
			constExpr();
			if (lexer::getTokenType() == COLON)
			{
				lexer::getToken();
				stat();
			}
		}
		outputFile << "<情况子语句>" << endl;
	}

	static void defaultStat()
	{
		if (lexer::getTokenType() == DEFAULTTK)
		{
			lexer::getToken();
			if (lexer::getTokenType() == COLON)
			{
				lexer::getToken();
				stat();
			}
		}
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
		int ret = stoi(lexer::curToken->getStr());
		if (lexer::getTokenType() == INTCON)
		{
			lexer::getToken();
			outputFile << "<无符号整数>" << endl;
			return ret;
		}
		else
		{
			parseError();
		}
		return -1;
	}

	static char character()
	{
		char ret = lexer::curToken->getStr().at(0);
		if (lexer::getTokenType() == CHARCON)
		{
			lexer::getToken();
			return ret;
		}
		else
		{
			parseError();
		}
		return -1;
	}

	static void stringCon()
	{
		if (lexer::getTokenType() == STRCON)
		{
			lexer::getToken();
			outputFile << "<字符串>" << endl;
		}
		else
		{
			parseError();
		}
	}

	static void expr()
	{
		if (lexer::getTokenType() == PLUS ||
			lexer::getTokenType() == MINU)
		{
			lexer::getToken();
		}
		term();
		while (lexer::getTokenType() == PLUS ||
			   lexer::getTokenType() == MINU)
		{
			lexer::getToken();
			term();
		}
		outputFile << "<表达式>" << endl;
	}

	static void term()
	{
		factor();
		while (lexer::getTokenType() == MULT ||
			   lexer::getTokenType() == DIV)
		{
			lexer::getToken();
			factor();
		}
		outputFile << "<项>" << endl;
	}

	static void factor()
	{
		if (isStat() == RETURNCALLSTAT)
		{
			returnCallStat();
			outputFile << "<因子>" << endl;
			return;
		}
		if (lexer::getTokenType() == IDENFR)
		{ // Dimension 0 Left
			CheckUndefinedIdentifier;
			lexer::getToken();
			if (lexer::getTokenType() == LBRACK)
			{
				lexer::getToken();
				expr();
				if (lexer::getTokenType() == RBRACK)
				{// Dimension 1 Left
					lexer::getToken();
					if (lexer::getTokenType() == LBRACK)
					{
						lexer::getToken();
						expr();
						if (lexer::getTokenType() == RBRACK)
						{ // Dimension 2 Left
							lexer::getToken();
							outputFile << "<因子>" << endl;
							return;
						}
						else
						{
							parseError();
						}
					}
					outputFile << "<因子>" << endl;
					return;
				}
				else
				{
					parseError();
				}
			}
			outputFile << "<因子>" << endl;
			return;
		}
		else if (lexer::getTokenType() == PLUS ||
				 lexer::getTokenType() == MINU ||
				 lexer::getTokenType() == INTCON)
		{
			integer();
			outputFile << "<因子>" << endl;
			return;
		}
		else if (lexer::getTokenType() == CHARCON)
		{
			character();
			outputFile << "<因子>" << endl;
			return;
		}
		else if (lexer::getTokenType() == LPARENT)
		{
			lexer::getToken();
			expr();
			if (lexer::getTokenType() == RPARENT)
			{
				lexer::getToken();
				outputFile << "<因子>" << endl;
				return;
			}
		}
	}

	static void relationOp()
	{
		switch (lexer::getTokenType())
		{
		case LSS:
		case LEQ:
		case GRE:
		case GEQ:
		case EQL:
		case NEQ:
			lexer::getToken();
			return;
		default:
			parseError();
		}
	}

	static void constExpr()
	{
		switch (lexer::getTokenType())
		{
		case INTCON:
		case PLUS:
		case MINU:
			integer();
			outputFile << "<常量>" << endl;
			return;
		case CHARCON:
			character();
			outputFile << "<常量>" << endl;
			return;
		default:
			break;
		}
		parseError();
	}