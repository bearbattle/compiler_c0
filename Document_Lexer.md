# 词法分析阶段设计文档

18373463 熊胡超

## 设计思路

### 文法分析

题中给出了相应的表格，这份表格给出了大部分的文法单元。

| 单词名称 | 类别码  | 单词名称 | 类别码    | 单词名称 | 类别码 | 单词名称 | 类别码  |
| -------- | ------- | -------- | --------- | -------- | ------ | -------- | ------- |
| 标识符   | IDENFR  | else     | ELSETK    | -        | MINU   | =        | ASSIGN  |
| 整形常量 | INTCON  | switch   | SWITCHTK  | *        | MULT   | ;        | SEMICN  |
| 字符常量 | CHARCON | case     | CASETK    | /        | DIV    | ,        | COMMA   |
| 字符串   | STRCON  | default  | DEFAULTTK | <        | LSS    | (        | LPARENT |
| const    | CONSTTK | while    | WHILETK   | <=       | LEQ    | )        | RPARENT |
| int      | INTTK   | for      | FORTK     | >        | GRE    | [        | LBRACK  |
| char     | CHARTK  | scanf    | SCANFTK   | >=       | GEQ    | ]        | RBRACK  |
| void     | VOIDTK  | printf   | PRINTFTK  | ==       | EQL    | {        | LBRACE  |
| main     | MAINTK  | return   | RETURNTK  | !=       | NEQ    | }        | RBRACE  |
| if       | IFTK    | +        | PLUS      | ：       | COLON  |          |         |

按照这份表格中的单词名称，可以将这些语法成分划分为以下几类：

| 类别         | 类别码                                                       |
| ------------ | ------------------------------------------------------------ |
| 标识符       | IDENFR                                                       |
| 整型常量     | INTCON                                                       |
| 字符常量     | CHARCON                                                      |
| 字符串       | STRCON                                                       |
| 关键字       | CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK, ELSETK, SWITCHTK, CASETK, DEFAULTTK, WHILETK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK |
| 单字符分界符 | PLUS, MINU, MULT, DIV, LSS, GRE, COLON, ASSIGN, SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE |
| 双字符分界符 | LEQ, GEQ, EQL, NEQ                                           |

这些不同的语法成分类别分别对应着不同的状态图，因此可以使用状态机来进行简单的匹配。

### 构造思路

词法分析程序使用状态机来对语法成分进行识别。在构造相关的程序过程中，考虑到词法分析程序需要将结果返回调用者，因此其返回值应具有相应的标识性。因此，需要构造其返回值对应的结构，在这里，本人使用了`Token`类来完成这一工作。`Token`类应具有基本的数据存储功能，同时，题目也要求对识别内容进行输出，因此还应当实现该类的输出。

正如上文所述，程序主题将使用针对不同类的状态机构造的自动机来完成对不同的语法成分的识别。
在开始状态，首先要读进一个字符，可通过调用一个读字符子程序来完成。若读入一个空字符就跳过它，再继续读字符，直到读进一个非空字符为止。接下去的工作是根据所读进的非空字符转到相应的程序段进行处理。
在标识符状态，在识别并组合成标识符以后，还必须加入一些动作，以便查保留字表，确定是保留字还是用户定义的标识符，并分别按情况输出单词。
在整数状态，在组数的同时还要做数字字符到十进制数值的转换。
在单字符分界符状态，只需输出其内部的类别编码即可。

词法分析程序在遇到不合法字符时要进行错误处理，其主要工作是显示或打印错误信息，并跳过这个字符，然后转开始状态继续识别和组合下一个单词。

需要注意的是，在词法分析过程中，为了判别是否已读到单词的右端符号，有时需要向前多读一个字符，例如在标识符和无符号整数等状态。因此，词法分析程序在返回调用程序以前应将读字符指针后退一个字符。

## 实现过程

### Token 类的构造

上文中提到，`Token`类应具有数据存储功能和输出功能。在C++的程序实现中，使用`std::string`直接对`Token`的内容进行存储。同时，为了标识`Token`的类别，使用`TokenType`这一枚举类来实现。为了方便地实现输出，本人采用了重载`<<`运算符这一方法。最终效果如下：

```C++
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

// Token Class
class Token {
private:
    string str;
    TokenType type;
public:
    friend ostream& operator<<(ostream& out, const Token& A);
    Token();
    Token(string str, TokenType type);
    Token(char ch_i, TokenType type);
};
```

同时，在具体实现`Token`类的输出过程中，需要实现将`TokenType`转化为`std::string`的过程。本人采用了`std::map<TokenType, std::string>`的映射这一方法，代码如下：

```c++
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

```

### 一些变量和过程

为了实现相应的功能，在文件中声明并实现了一些变量和过程，其声明如下：

```c++
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

void getChar();

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

```

为了避免与之后的文件内容发生冲突，暂且将这些函数声明为静态函数。

### getToken() 函数的实现

本次作业的核心任务是实现`getToken()`这一函数，因此，对该函数进行设计是必要的。

需要注意的是，该函数应当返回相应的`Token`对象，为了实现功能，将该函数的返回值类型确定为`Token *`。在此基础上，函数内部使用`new`来返回指向新对象的指针。最终，我们得到了以下的函数原型：

```c++
Token* getToken();
```

该函数的主要实现思路同71-73页，仅对一些过程进行了细微的调整，以支持<字符常量>，<字符串>以及其他一些双字符分界符。在出错或者文件结束时，该函数将会返回`NULL`。更具体的实现详见代码，本文不再赘述。

## 方案修改

在完成了第一版设计之后。使用样例对数据进行了测试，通过了测试，遂进行提交，发现部分样例没有通过，因而对实现进行修改。

### Bug Fix 0: 忽略了空格和换行符

在样例测试过程中，出现了没有识别`INTTK`的问题。在后续的调试过程中，发现了空白和新行被跳过。

查询资料得知`>>`操作符会过滤空白符号，遂修改`getChar()`方法。

### Bug Fix 1: 返回空的IDENFR Token

在匹配过程中由于出现了`'\r'`字符，在对`isNewline()`进行判断的时候没有考虑到，因而返回了空的`Token`内容。

修改`isNewline()`方法，解决了此类问题。

### Bug Fix 2: 在末尾存在多余的IDENFR Token

匹配过程中文件结束，没有进行判断，使得出现多余`Token`。

修改`getToken()`方法，改进了对文件结束符的判断。