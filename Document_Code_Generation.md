# 代码生成阶段设计文档

18373463 熊胡超

## 设计思路

由于从原C语言代码直接翻译到MIPS代码较难实现，因此选择使用中间代码这一过渡工具来辅助实现代码生成。

### 中间代码的设计

中间代码的设计参考了上届学长的思路，主要有以下的几类：

| 名称         | 代码          | 类名             | 备注                               |
| ------------ | ------------- | ---------------- | ---------------------------------- |
| 赋值语句     | `ASS_MID`     | `AssignMid`      |                                    |
| 读语句       | `READ_MID`    | `ReadMid`        |                                    |
| 写语句       | `WRITE_MID`   | `WriteMid`       |                                    |
| 函数调用语句 | `CALL_MID`    | `CallMid`        | 需要将函数参数压入栈中             |
| 返回语句     | `RET_MID`     | `ReturnMid`      | 需要返回值填入`$v0`寄存器          |
| 分支语句     | `BRA_MID`     | `BranchMid`      |                                    |
| 跳转语句     | `JMP_MID`     | `JumpMid`        |                                    |
| 标号（语句） | `LAB_MID`     | `LabelMid`       | 见后文                             |
| 函数语句     | `FUN_MID`     | `FunctionMid`    | 声明函数定义开头，存储函数返回地址 |
| 函数结束语句 | `FUN_END_MID` | `FunctionEndMid` | 同返回语句，另有定界功能           |

可以看出，本次设计的中间代码相较于原类C语法，删减了大量内容，从而更加贴近MIPS代码。这样就极大地简化了从中间代码向MIPS翻译的过程。

所有的中间代码都扩展自`MidCode`类，通过基类中的`MidType`进行判断。

### 变量的相关设计

C语言的变量对应着内存中的对象。同时，为了实现表达式的相关计算和函数调用、数组下标等功能，需要对在中间代码中使用的变量进行设计。下表给出了本次代码生成的过程中对C语言变量、中间变量、函数返回值、数组等的相关处理中涉及的变量设计。

| 名称     | 代码      | 类名        | 备注                                                         |
| -------- | --------- | ----------- | ------------------------------------------------------------ |
| 临时变量 | `TEMPVAR` | `TempVar`   | 在计算中主要使用的中间变量                                   |
| 基本变量 | `BASEVAR` | `Var`       | C语言中的普通变量（非数组）                                  |
| 返回变量 | `RETVAR`  | `RetVar`    | 有返回值的函数调用，<br />其返回值存储在这个变量中           |
| 常量     | `CONVAR`  | `ConstVar`  | 字面值和C语言中的的常量<br />（被替换为对应的值）            |
| 数组变量 | `ARRAY`   | `ArrayVar`  | 为避免寄存器冲突，其下标<br />**只能**为`ConstVar`或`TempVar` |
| 字符串   | `STRING`  | `StringVar` |                                                              |

由于决定直接使用内存而尽量避免使用寄存器，本次的变量相关设计并不复杂。

所有的变量都继承自`VarBase`，通过基类中的`VarType`进行判断。

### 标号的相关设计

`Label`是汇编程序的重要概念，其为分支语句和跳转语句提供了基本的入手方式，进而可以实现更高级的操作（例如循环、函数调用）。本次的代码生成作业中有很大一部分是关于标号的设计。详情参见下表：

| 名称         | 类名          | 备注                                          |
| ------------ | ------------- | --------------------------------------------- |
| `If`标号     | `IfLabel`     | 包含一个`ElseLabel`和一个`EndLabel`           |
| `Else`标号   | `ElseLabel`   | 并不单独使用，见上                            |
| 开始标号     | `BeginLabel`  | 存在于`LoopLabel`中                           |
| 结束标号     | `EndLabel`    | 存在于`LoopLabel`，`IfLabel`和`SwitchLabel`中 |
| 循环标号     | `LoopLabel`   | 包含一个`BeginLabel`和一个`EndLabel`          |
| `Switch`标号 | `SwitchLabel` | 包含一个`EndLabel`和若干个`CaseLabel`         |
| `Case`标号   | `CaseLabel`   | 用于`case`语句分支，也用于`default`           |

所有的标号都继承自`BaseLabel`。由于标号仅需要输出，因此不需要指定相关域来进行子类型的判断。

## 实现和改进

由于时间的关系，这里只展示部分代码。

> Talk is cheap. Show me the code.
>
> <p align="right">——Linus Torvalds</p>

### 中间代码、变量和标号

```cpp
//// MidCode.h

#include <ostream>
#include <queue>
#include <map>
#include <set>
#include <stack>
#include "SymbolTable.h"
#include "Token.h"

using namespace std;

enum VarType
{
    TEMPVAR,
    BASEVAR,
    RETVAR,
    CONVAR,
    ARRAY,
    STRING
};

class VarBase
{
public:
    VarType varType;

    explicit VarBase(VarType varType);

    virtual void out(ostream& os) const = 0;
};

class ConstVar : public VarBase
{
public:
    int value;

    explicit ConstVar(int value);

    void out(std::ostream& os) const override;
};

class TempVar : public VarBase
{
public:
    static int count;
    int id;
    int offset;

    TempVar();

    void out(ostream& os) const override;
};

class RetVar : public VarBase
{
public:
    RetVar();

    void out(std::ostream& os) const override;
};

class Var : public VarBase
{
public:
    SymTabEntry* symTabEntry;

    explicit Var(SymTabEntry* symTabEntry);

    void out(std::ostream& os) const override;
};

class ArrayVar : public VarBase
{
public:
    SymTabEntry* symTabEntry;
    VarBase* index;

    ArrayVar(SymTabEntry* symTabEntry, VarBase* subscript, VarBase* subscript1 = nullptr);

    void out(std::ostream& os) const override;
};

class StringVar : public VarBase
{
public:

    explicit StringVar(const string& str);

    void out(std::ostream& os) const override;

    static set<StringVar*> stringVars;

    const string& getStr() const;

    const string& getLabel() const;

private:
    static int count;
    string str;
    string label;
    int id;
};

class BaseLabel
{
public:
    explicit BaseLabel(bool inc = true);

    virtual string label() const;

protected:
    static int count;
    int id;
};

class IfLabel;

class ElseLabel;

class EndLabel;

class IfLabel : public BaseLabel
{
public:
    IfLabel();

    string label() const override;

    BaseLabel* elseLabel;

    BaseLabel* endLabel;
};

class ElseLabel : public BaseLabel
{
public:
    ElseLabel();

    string label() const override;
};

class BeginLabel : public BaseLabel
{
public:
    BeginLabel();

    string label() const override;

};

class EndLabel : public BaseLabel
{
public:
    EndLabel();

    string label() const override;

};

class LoopLabel : public BaseLabel
{
public:
    LoopLabel();

    BaseLabel* beginLabel;
    BaseLabel* endLabel;
};

class SwitchLabel;

class CaseLabel;

class CaseLabel : public BaseLabel
{
public:
    explicit CaseLabel(SwitchLabel* switchLabel, long id);

    SwitchLabel* switchLabel;

    string label() const override;

private:
    long id;
};

class SwitchLabel : public BaseLabel
{
public:
    explicit SwitchLabel(VarBase* caseVar);

    VarBase* caseVar;
    stack<CaseLabel*> caseLabels;

    CaseLabel* curCaseLabel();

    CaseLabel* nextCaseLabel();

    EndLabel* endLabel;

    string label() const override;
};

enum MidType
{
    ASS_MID,
    CALL_MID,
    RET_MID,
    BRA_MID,
    JMP_MID,
    LAB_MID,
    FUN_MID,
    FUN_END_MID,
    WRITE_MID,
    READ_MID
};

class MidCode
{
public:
    explicit MidCode(MidType type);

    virtual void out(ostream& os) const = 0;

    MidType getType() const;

private:
    MidType type;

};

enum MidOp
{
    ADD_OP, SUB_OP, MUL_OP, DIV_OP,
    LSS_OP, LEQ_OP, GRE_OP, GEQ_OP, EQL_OP, NEQ_OP,
    ILLEGAL_OP,
};

extern map<TokenType, MidOp> MidOpMap;

extern map<MidOp, string> op2Branch;

class AssignMid : public MidCode
{
public:
    AssignMid(MidOp op, VarBase* left, VarBase* des);

    AssignMid(MidOp op, VarBase* left, VarBase* right, VarBase* des);

    MidOp op;
    VarBase* left, * right, * des;

    void out(ostream& os) const override;
};

class CallMid : public MidCode
{
public:

    explicit CallMid(SymTabEntry* symTabEntry);

    SymTabEntry* symTabEntry;
    vector<VarBase*> valTable;

    void out(ostream& os) const override;
};

class ReturnMid : public MidCode
{
public:
    ReturnMid();

    explicit ReturnMid(VarBase* ret);

    VarBase* ret;
    bool isVoid;

    void out(ostream& os) const override;
};

class BranchMid : public MidCode
{
public:
    MidOp op;
    VarBase* left, * right;
    BaseLabel* label;

    BranchMid(MidOp op, VarBase* left, VarBase* right, BaseLabel* label, bool reverse = true);

    void out(ostream& os) const override;
};

class JumpMid : public MidCode
{
public:
    BaseLabel* label;

    explicit JumpMid(BaseLabel* label);

    void out(ostream& os) const override;
};

class LabelMid : public MidCode
{
public:
    BaseLabel* label;

    explicit LabelMid(BaseLabel* label);

    void out(ostream& os) const override;
};

class FunctionMid : public MidCode
{
public:
    SymTabEntry* symTabEntry;

    explicit FunctionMid(SymTabEntry* symTabEntry);

    void out(ostream& os) const override;
};

class ReadMid : public MidCode
{
public:
    Var* var;

    explicit ReadMid(Var* var);

    void out(std::ostream& os) const override;
};

class WriteMid : public MidCode
{
public:
    VarBase* var;
    bool isChar;
    bool isString;

    WriteMid(VarBase* var, bool isChar = false);

    WriteMid();

    void out(std::ostream& os) const override;

private:
    static StringVar* newLine;
};

class FunctionEndMid : public MidCode
{
public:
    FunctionEndMid();

    void out(std::ostream& os) const override;
};

extern vector<MidCode*> midCodes;

extern ofstream midCodeFileInit;
```

### 中间代码生成

#### 主要难点

- 表达式的处理

	对表达式的处理采用了属性制导的方式，顶层的`expr()`传递`VarBase`的指针引用来实现属性的继承与综合。

	- 相关方法

		```cpp
		static BaseType expr(VarBase*& var); // Native: +|-, term()
		static BaseType term(VarBase*& var); // Native: factor(), * /
		static BaseType factor(VarBase*& var); // Native: <IDENFR>, [], integer(), character(), returnCallStat()
		static MidOp relationOp(); // Native: < | <= | > | >= | != | ==
		static BaseType constExpr(VarBase*& var);
		```

		在各函数中可能会生成赋值语句。

- `switch-case`语句的处理

	`switch-case`语句的处理的难点在于要实现多次的分支比较与跳转，在实现中，采用了以下思想：在当前分支的开头对变量和给定常量进行比较，如果不相等则跳转到下一分支在进行判断，直到到达默认分支。

	- 相关方法

		```cpp
		// caseStat: expr()
		static void caseTable(BaseType caseType, SwitchLabel* switchLabel); // Native: caseSubStat()
		static void caseSubStat(BaseType caseType, SwitchLabel* switchLabel); // Native: constExpr(), stat()
		static void defaultStat(SwitchLabel* switchLabel); // Native: stat()
		```

		其中

		```cpp
		static void caseSubStat(BaseType caseType, SwitchLabel* switchLabel)
		{
		    midCodes.push_back(new LabelMid(switchLabel->curCaseLabel()));
		    // ...
		    midCodes.push_back(new BranchMid(NEQ_OP,
		        switchLabel->caseVar, caseSubVar,
		        switchLabel->nextCaseLabel(), false));
		    stat();
		    midCodes.push_back(new JumpMid(switchLabel->endLabel));
		    outputFile << "<情况子语句>" << endl;
		}
		```

		```cpp
		static void defaultStat(SwitchLabel* switchLabel)
		{
		    midCodes.push_back(new LabelMid(switchLabel->curCaseLabel()));
		    ACCEPT(DEFAULTTK);
		    ACCEPT(COLON);
		    stat();
		    outputFile << "<缺省>" << endl;
		}
		```

- 循环语句的处理

	难点主要在于把握好标号和跳转语句的插入位置和顺序。`while`循环的示例如下：

	```cpp
		midCodes.push_back(new LabelMid(loopLabel->beginLabel)); // 先生成开始标号
	    lexer::getToken();
	    ACCEPT(LPARENT);
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
	    midCodes.push_back(new BranchMid(op, leftVar, rightVar, loopLabel->endLabel)); // 再生成分支语句
	    stat(); // 语句的中间代码生成
	    midCodes.push_back(new JumpMid(loopLabel->beginLabel)); // 跳转语句
	    midCodes.push_back(new LabelMid(loopLabel->endLabel)); // 结束标号
	```

### 数组相关

最好的解决方案是将二位数组转化为一维数组，代码如下：

```cpp
ArrayVar::ArrayVar(SymTabEntry* symTabEntry, VarBase* subscript, VarBase* subscript1) : VarBase(ARRAY),
symTabEntry(symTabEntry)
{
    if (subscript1 == nullptr)// One Dimension Array
    {
        // ...
    }
    else if (subscript1->varType == CONVAR && subscript->varType == CONVAR)
    {
        auto* sub = (ConstVar*)subscript;
        auto* sub1 = (ConstVar*)subscript1;
        index = new ConstVar(sub->value * symTabEntry->getLength(2) + sub1->value);
    }
    else
    {
        auto* t1 = new TempVar();
        midCodes.push_back(new AssignMid(MUL_OP, subscript,
            new ConstVar(symTabEntry->getLength(2)),
            t1));
        auto* t2 = new TempVar();
        midCodes.push_back(new AssignMid(ADD_OP, t1,
            subscript1,
            t2));
        index = t2;
    }
}
```