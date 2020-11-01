# 错误处理阶段设计文档

18373463 熊胡超

## 设计思路

### 错误分析

本次作业中需要处理存在语法和语义错误，进行错误的局部化处理并输出错误信息。

在本次作业中出现的错误可以大致分为语法错误和语义错误，其中类别`a`的错误应当在词法分析中进行判断，而其他的错误应当在语法分析程序中进行判断。

### 符号表管理

本次作业再次涉及到符号表的相关管理。类别`b`,`c`是直接与符号表中的表项存在与否的判断。这两者可以直接放在符号表类中进行判断（在设计中`b`的判断被放在了向符号表中添加时进行判断，`c`则是在进行语法分析的时候进行判断）。

类别`d`，`e`是符号表中函数表项的判断，设计中将其封装在符号表中，使用<值参数表>作为方法参数进行判断。类别`g`,`h`需要符号表和<返回语句>的配合。

类别`j`是符号表中常量的判断，也需要配合<语句>中的相关判断。

类别`n`则是符号表中数组表项的相关判断，实际中将其置于<变量定义及初始化>中处理。

### 属性继承与综合

在本次错误处理中需要判断<常量>、<表达式>、<项>、<因子>的相关属性，即其返回类型。`e`,`f`,`h`,`i`,`o`都需要进行属性的继承或综合才能进行判断。

另外的，在`d`,`e`,`g`,`h`中需要对当前函数进行属性的相关处理。

### 具体语法分析子程序相关

类别`f`需要在<条件>中判断，类别`i`需要在<语句>中涉及对数组的引用时进行判断，类别`j`需要在<赋值语句>和<写语句>中对标识符进行判断，类别`k`,`l`,`m`需要在需要在各自的位置上进行判断，类别`o`,`p`需要在<情况语句>、<情况表>、<情况子语句>中进行判断。

### 错误类的设计

```c++
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
```

实际使用时，会直接调用`errList.emplace_back(...)`来向错误列表中添加新的错误。

输出也直接重载了`operator<<`来实现。

## 实现及改进

### 预读的处理

在之前的设计中使用了`preFetch`这样的方法来对语法成分进行简单的判断。但是在此次设计中需要应对可能出现的错误，因此这一部分需要重写来应对可能出现的问题。

例如，在<变量定义>中可能会出现缺少右方括号`]`和分号`;`的情况出现，因此在进行判断的时候不能以这些符号是否出现来作为衡量的指标。

### 符号表的更新

本次设计的符号表进行了大幅度的更新，为了适应出现的新的问题，符号表的头文件现在修改如下：

```c++
#include <string>
#include <map>
#include <vector>
#include <deque>

using namespace std;

enum BaseType
{
    UNDEF, INT, CHAR, VOID
};

class SymTabEntry;

class SymTab;

class Param;

class ParamTab;

class SymTabEntry
{
private:
    string _name;
    BaseType _baseType;
    bool _isFunc;
    bool _isConst;
    int _dimension;
    int _line;
    int _initVal;
    SymTab* _symTab;
    ParamTab* _paramTab;
    int _length[2]{ 0, 0 };
public:
    SymTabEntry(const string& name, BaseType baseType, int line);

    const string& getName() const;

    BaseType getBaseType() const;

    int getLine() const;

    bool isFunc() const;

    void setFunc(bool isFunc);

    bool isConst() const;

    void setConst(bool isConst);

    int getDimension() const;

    void setDimension(int dimension);

    SymTab* getSymTab() const;

    void setSymTab(SymTab* symTab);

    ParamTab* getParamTab() const;

    void setParamTab(ParamTab* paramTab);

    int getInitVal() const;

    void setInitVal(int initVal);

    int getLength(int dimension) const;

    void setLength(int dimension, int length);
};

class SymTab
{
private:
    map<string, SymTabEntry*> _symTab;
    int _layer;
public:
    explicit SymTab(int layer);

    SymTabEntry* find(const string& str) const;

    bool exist(const string& str, bool curLayer = false);

    void addEntry(const string& name, SymTabEntry* entry, bool override = false);
};

class Param
{
private:
    BaseType type;
    string name;
public:
    Param(BaseType type, const string& name);

    BaseType getType() const;
};

class ParamTab
{
private:
    vector<Param> params;
public:
    void addParam(BaseType type, const string& name);

    void checkParams(const vector<BaseType>& valTable, int line);
};

extern deque<SymTab*> SymTabs;

void initSymTabs();

SymTab*& getGSymTab();
```

主要变化就是新增了`Param`和`ParamTab`这两个为函数服务的类。另外，为了应对可能出现的未定义的变量，在`BaseType`枚举中新增了`UNDEF`来表示未定义的变量类型。 

### 一些宏定义

```cpp
#define SemicnExpected
#define RparentExpected
#define RbrackExpected
#define SymTabAddFunc
#define SymTabAddConst
#define SymTabAddVar
#define InitVarArray(dimension)
#define CheckArray(dimension)
#define CheckConstType
#define CheckUndefinedIdentifier
#define CheckConstAssign
#define GetCurFuncParamTab
#define GetCurBaseType(baseType)
#define CheckCaseType
```

这些宏定义的命名都较好地体现了其发挥的作用，因此这里对其作用不再赘述。

### 属性的继承和综合

属性的继承与综合很大程度上依赖了`SymbolTable.h`中定义的`enum BaseType`。主要修改了一下函数的返回类型：

```c++
static BaseType typeSpec();
static BaseType returnCallStat();
static void caseTable(BaseType caseType);
static void caseSubStat(BaseType caseType);
static BaseType expr();
static BaseType term();
static BaseType factor();
static BaseType constExpr();
```

<条件语句>相关的内容在前面的设计思路中已经提到了相关的思路，具体实现不再赘述。

在<表达式>、<项>、<因子>的处理上需要进行属性的综合，在这个过程中需要区别`INT`和`CHAR`。

### 使用 `goto` 语句

非常抱歉，在本次程序设计的过程中，为了降低程序代码的冗杂度，使用了`goto`语句。