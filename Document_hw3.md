# 语法分析阶段设计文档

18373463 熊胡超

## 设计思路

### 文法分析

由于给出的文法过于复杂，且需要按照给定的文法定义在Token流中插入语法分析的结果，因此无法也无需对文法进行改写。

对于该文法，需要注意的是，部分同级语法成分的FIRST与其他语法成分相同，因此需要在处理这些FIRST集相同的语法成分时进行鉴别，本实现中采取了“预读”这一方法来对这些语法成分进行区分。另外，在一些可能重复出现的语法成分中也需要进行“预读”来判断接下来是否应该继续进行当前的语法分析。特殊的，在对<有返回值函数调用语句>和<无返回值函数调用语句>是需要查询符号表。下面详细说明。

#### <程序>级：<无返回值的函数定义>和<主函数>

两个语法成分的FIRST={`void`}，故需要进行预读区分。

区分的部分在`curToken`后1个，前者为`IDENFR`，后者为`MAINTK`。

#### <变量声明>级：{<变量定义>;}的判断

需要进行预读以判断接下来的是否为变量定义。

标志的部分在`curToken`后2个，如果`nextTokenType`为`SEMICN`,`COMMA`,`LBRACK`或`ASSIGN`则满足条件，否则结束<变量声明>级分析。

#### <变量定义>级：<变量定义无初始化>和<变量定义及初始化>

二者的主要区别在于是否会出现`ASSIGN`这一“初始化”的标志。因此，需要进行预读。

分阶段预读，先预读`curToken`后2个：如果是`ASSIGN`，则为<变量定义及初始化>；如果是`SEMICN`或者`COMMA`，则为<变量定义无初始化>；如果是`LBRACK`，则需要进一步判断，对`curToken`后5个重复上面的步骤；如果仍为`LBRACK`，则需要进一步判断，继续对`curToken`后8个重复上面的步骤，此时不应出现`LBRACK`，判断结束。

*需要注意的是，`C++`在`SWITCH-CASE`语句中不能声明新的变量。*

#### <语句列>和<语句>级：各类语句的判断

在语句中需要判断当前语句为何种语句。对<循环语句>、<条件语句><读语句>、<写语句>、<情况语句>、<空>、<返回语句>、<语句列>而言，其均具有不同的FIRST集，因此较好判断而对<有返回值函数调用语句>、<无返回值函数调用语句>、<赋值语句>这些具有相同FIRST集的语法成分则需要进一步判断。

标志的部分在`curToken`后2个，如果`nextTokenType`为`LBRACK`或`ASSIGN`即为<赋值语句>；如果其为`LPARENT`，则需要查找符号表来判断其是<有返回值函数调用语句>还是<无返回值函数调用语句>。

### 构造思路

语法分析程序依赖于词法分析，其主要功能是将Token流转化为相应的语法成分。本程序采取了递归下降分析法，对文法中的*每个要求输出*的非终结符号（语法成分）都编出一个子程序，以完成该给终结符号对应的语法成分的分析和识别任务。

在本次编程实现的过程中，采用了类似“广度优先”的方法来规范对程序源代码文件中的函数构造顺序的安排。

由于本次文法中带有回溯，因此需要在基础的递归下降方法中添加预读这一功能来规避这一回溯可能。

另外，上文中提到了对<有返回值函数调用语句>、<无返回值函数调用语句>的判断需要查询符号表。

## 实现过程

### 各递归子函数的声明与实现

基于“广度优先”的思想，本人的各递归子函数声明如下。

```c++
// Parser will start with program()

void program(); // 5

// program:
static void constDec(); // 12
static void varDec(); // 15
static void returnFuncDef(); // 22
static void nonReturnFuncDef(); // 27
static void mainFunc(); // Native: compoundStat()

// constDec:
static void constDef(); // Native: int, <IDENFR>, integer(), char, character()

// varDec:
static void varDef(); // 18

// varDef:
static void varDefNoInit(); // Native: typeSpec(), <IDENFR>, [], unsignedInteger()
static void varDefWithInit(); // Native: typeSpec(), <IDENFR>, [], unsignedInteger(), constExpr()

// returnFuncDef:
static SymbolTableEntry* defHead(); // Native: int|char <IDENFR>
static void paramTable(); // 32
static void compoundStat(); // 35

// nonReturnFuncDef:
//static void defHead(); // Native: void <IDENFR>
//static void paramTable(); // 32
//static void compoundStat(); // 35

// paramTable: <IDENFR>
static void typeSpec(); // Native: int|char

// compoundStat:
//static void constDec();
//static void varDec();
static void stats(); // 40

// stats:
static void stat(); //43

// stat:
static void loopStat(); // 55
static void ifStat();// Native: condition(), stat()
static void returnCallStat();// 61
static void nonReturnCallStat();// 61
static void assignStat();// Native: <IDENFR>, expr()
static void readStat();// Native: scanf, <IDENFR>
static void writeStat();// Native: printf, <STRCON>, expr()
static void caseStat();// 64
static void returnStat();// Native: expr()
// { static void stats() } // 60

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
static void stringCon(); // Native: <STRCON>
static void expr(); // Native: +|-, term()
static void term(); // Native: factor(), * /
static void factor(); // Native: <IDENFR>, [], integer(), character(), returnCallStat()
static void relationOp(); // Native: < | <= | > | >= | != | ==
static void constExpr(); // Native: <INTCON>, <CHARCON>
```

之后，对这些函数分别编写即可。

另外，对于一些需要进行预读判断的复杂过程，同样创建了相关的函数：

```c++
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
```

此外，为了方便查找程序错误，定义了`error()`宏：

```C++
#define error() do { \
    cout << "Parser Error! At " << __FUNCTION__ << ":" << __LINE__ << endl; \
} while (false)
```

### Lexer 的改造

由于语法分析需要调用词法分析程序来获取Token，同时也需要实现预读等功能，因此需要对 Lexer 封装改造，效果如下：

```c++
//// lexer.h ////

namespace lexer {
    extern Token* curToken;
    extern deque<Token*> afterWards;
    Token* preFetch(int amount);
    Token* _getToken(); // 原getToken()函数
    void getToken();
    TokenType getTokenType();
}

//// lexer.cpp ////
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
```

经过改造后，语法分析程序使用`lexer::`命名空间中的函数来进行相关操作。

### 符号表的构造与使用

#### 构造

为了实现对<有返回值函数调用语句>、<无返回值函数调用语句>的判断，构造一个简单的符号表：

```c++
//// SymbolTable.h ////
#include <string>
#include <map>
#include <deque>

using namespace std;

enum BaseType {
    INT, CHAR, VOID
};

class SymbolTableEntry {
private:
    string _name;
    BaseType _baseType;
    bool _isFunc;
    bool _isConst;
    int _dimension;
public:
    SymbolTableEntry(string name, BaseType baseType);

    const string& getName() const;

    void setName(const string& name);

    BaseType getBaseType() const;

    void setBaseType(BaseType baseType);

    bool isFunc() const;

    void setFunc(bool isFunc);

    bool isConst() const;

    void setConst(bool isConst);

    int getDimension() const;

    void setDimension(int dimension);
};

extern deque<map<string, SymbolTableEntry*>> SymTabs;

void initSymTabs();
map<string, SymbolTableEntry*>& getGSymTab();
```

符号表的基本实现是栈式符号表。使用 `initSymTabs`来初始化符号表。使用`getGSymTab()`来获取全局符号表。

#### 使用

##### 符号表的插入

在`static void returnFuncDef();` 和`static void nonReturnFuncDef();` 中进行符号表的插入操作。

每次识别到函数的声明头（返回类型、标识符、参数表）时，即可进行符号表的插入操作。

```C++
//// returnFuncDef() ////
auto* newEntry = defHead();
/* Some other Stuff ( like paramTable() )*/
newEntry->setFunc(true);
getGSymTab()[newEntry->getName()] = newEntry;

//// nonReturnFuncDef() ////
assert(lexer::getTokenType() == VOIDTK);
/* Some other Stuff ( like paramTable() )*/
auto* newEntry = new SymbolTableEntry(funcName, VOID);
newEntry->setFunc(true);
getGSymTab()[funcName] = newEntry;
```

需要注意的是，本次保证了输入文件的正确性。在接下来的过程中，这个过程需要检查符号表中是否已存在相应的表项。

##### 符号表的读取

在`static StatType isStat()` 中进行符号表的查询操作。

```c++
static StatType isStat() {
    Token* next;
    TokenType nextType;
    BaseType retType;
    string id;
    SymbolTableEntry* ptr;
    switch (lexer::getTokenType()) {
/* Some other Stuff ( like identifying loopStat, etc. )*/
            case IDENFR:
                id = lexer::curToken->getStr();
                next = lexer::preFetch(1);
                nextType = next->getType();
                if (nextType == ASSIGN
                    || nextType == LBRACK) {
                    return ASSIGNSTAT;
                }
                else if (nextType == LPARENT) {
                    ptr = getGSymTab().at(id);  // <<<<<<<<<<<<<<<<<<<<<< HERE
                    if (ptr->isFunc()) {
                        retType = ptr->getBaseType();
                        if (retType == VOID) {
                            return NONRETURNCALLSTAT;
                        }
                        else if (retType == INT || retType == CHAR) {
                            return RETURNCALLSTAT;
                        }
                    }
                    else {
                        error();
                    }
                }
                else {
                    error();
                }
            break;
/* Some other Stuff ( like identifying loopStat, etc. )*/
```

需要注意的是，本次保证了输入文件的正确性。在接下来的过程中，这个过程需要检查符号表中是否已存在相应的表项。

## 方案修改

在完成了第一版设计之后。使用样例对数据进行了测试，未通过了测试，因而对实现进行修改。

### Bug Fix 1: Token 的输出与更新

在获取新的`Token`时，会直接将该`Token`输出吗？在前期设计时，我的回答是：“是的。”但在随后的样例测试中，我发现这样无法解决`varDef()`之后的分号。在询问了同学之后，将其修改为`getToken()`会先输出当前值，随后进行更新。这样就解决了相关的问题。

### Bug Fix 2: 输出不全

未对<字符串>输出语法成分，已修复。

### Bug Fix 3: 未定义的函数符号

对函数自身递归调用的测试用例，早期设计查询符号表时会出现未捕获的异常。

早期实现在完成识别整个<函数>之后才会将相应的表项插入符号表，更新后在函数的声明头（返回类型、标识符、参数表）时识别完毕后即构造并插入新表项，已修复。

### Bug Fix 4: 未仔细研读文法造成的问题

细心看一遍文法定义，对着测试用例改进即可。