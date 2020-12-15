//
// Created by chao4 on 2020/11/14 0014.
//

#ifndef COMPILER_0_MIDCODE_H
#define COMPILER_0_MIDCODE_H

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

#define ARRAYSUPPORT
#define FUNCTIONSUPPORT

#endif //COMPILER_0_MIDCODE_H
