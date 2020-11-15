//
// Created by chao4 on 2020/11/14 0014.
//

#include "MidCode.h"

#include <utility>

VarBase::VarBase(VarType varType) : varType(varType)
{
}

ConstVar::ConstVar(int value) : VarBase(CONVAR), value(value)
{
}

void ConstVar::out(ostream& os) const
{
    os << value;
}

int TempVar::count = 0;

TempVar::TempVar() : VarBase(TEMPVAR)
{
    id = ++count;
    offset = count * 4;
}

void TempVar::out(ostream& os) const
{
    os << "$T" << id;
}

RetVar::RetVar() : VarBase(RETVAR)
{
}

void RetVar::out(ostream& os) const
{
    os << "$RET";
}

Var::Var(SymTabEntry* symTabEntry) : VarBase(BASEVAR), symTabEntry(symTabEntry)
{
}

void Var::out(ostream& os) const
{
    os << symTabEntry->getName();
}


ArrayVar::ArrayVar(SymTabEntry* symTabEntry, VarBase* subscript) : VarBase(ARRAY), symTabEntry(symTabEntry),
subscript(subscript)
{
}

void ArrayVar::out(ostream& os) const
{
    os << symTabEntry->getName() << "[";
    subscript->out(os);
    os << "]";
}

int BaseLabel::count = 0;

BaseLabel::BaseLabel() : id(++count)
{
}

string BaseLabel::label() const
{
    return string("label_") + to_string(id);
}

//LoopLabel::LoopLabel() : id(++count)
//{
//}
//
//string LoopLabel::headLabel() const
//{
//    return string("loop_head_") + to_string(id);
//}
//
//string LoopLabel::testLabel() const
//{
//    return string("loop_test_") + to_string(id);
//}
//
//string LoopLabel::bodyLabel() const
//{
//    return string("loop_body_") + to_string(id);
//}
//
//string LoopLabel::endLabel() const
//{
//    return string("loop_end_") + to_string(id);
//}

MidCode::MidCode(MidType type) : type(type)
{
}

MidType MidCode::getType() const
{
    return type;
}

static map<MidOp, string> op2Str = {
        { ADD_OP, " + " },
        { SUB_OP, " - " },
        { MUL_OP, " * " },
        { DIV_OP, " / " },
        { LSS_OP, " < " },
        { LEQ_OP, " <= " },
        { GRE_OP, " > " },
        { GEQ_OP, " >= " },
        { EQL_OP, " == " },
        { NEQ_OP, " != " }
};

void AssignMid::out(ostream& os) const
{
    if (right == nullptr)
    {
        des->out(os);
        os << " = ";
        if (op == SUB_OP)
        {
            os << "- ";
        }
        left->out(os);
    }
    else
    {
        des->out(os);
        os << " = ";
        left->out(os);
        os << op2Str[op];
        right->out(os);
    }
}

AssignMid::AssignMid(MidOp op, VarBase* left, VarBase* des) : MidCode(ASS_MID), op(op), left(left), right(nullptr),
des(des)
{
}

AssignMid::AssignMid(MidOp op, VarBase* left, VarBase* right, VarBase* des) : MidCode(ASS_MID), op(op),
left(left), right(right),
des(des)
{
}

void CallMid::out(ostream& os) const
{
    for (auto item : valTable)
    {
        os << "push ";
        item->out(os);
        os << endl;
    }
    os << "call " << symTabEntry->getName();
}

CallMid::CallMid(SymTabEntry* symTabEntry) : MidCode(CALL_MID), symTabEntry(symTabEntry)
{
}

ReturnMid::ReturnMid(VarBase* ret) : MidCode(RET_MID), ret(ret), isVoid(false)
{
}

ReturnMid::ReturnMid() : MidCode(RET_MID), ret(nullptr), isVoid(true)
{
}

void ReturnMid::out(ostream& os) const
{
    os << "return";
    if (!isVoid)
    {
        os << ' ';
        ret->out(os);
    }
}

BranchMid::BranchMid(MidOp op, VarBase* left, VarBase* right, BaseLabel* label) : MidCode(BRA_MID), op(op),
left(left),
right(right),
label(label)
{
}

static map<MidOp, string> op2Branch = {
        { LSS_OP, "blt " },
        { LEQ_OP, "ble " },
        { GRE_OP, "bgt " },
        { GEQ_OP, "bge " },
        { EQL_OP, "beq " },
        { NEQ_OP, "bne " }
};

void BranchMid::out(ostream& os) const
{
    os << op2Branch[op];
    left->out(os);
    os << ", ";
    right->out(os);
    os << ", ";
    os << label->label();
}

JumpMid::JumpMid(BaseLabel* label) : MidCode(JMP_MID), label(label)
{
}

void JumpMid::out(ostream& os) const
{
    os << label->label() << ":";
}

FunctionMid::FunctionMid(SymTabEntry* symTabEntry) : MidCode(FUN_MID), symTabEntry(symTabEntry)
{
}

void FunctionMid::out(ostream& os) const
{
    // TODO
}

ReadMid::ReadMid(Var* var) : MidCode(READ_MID), var(var)
{
}

void ReadMid::out(ostream& os) const
{
    os << "read ";
    var->out(os);
}

WriteMid::WriteMid(VarBase* var, bool isChar) : MidCode(WRITE_MID), var(var), isChar(isChar)
{
    isString = typeid(*var) == typeid(StringVar);
}

void WriteMid::out(ostream& os) const
{
    os << "write ";
    if (typeid(*var) == typeid(StringVar))
    {
        os << "string ";
    }
    else
    {
        os << (isChar ? "char " : "int ");
    }
    var->out(os);
}

FunctionEndMid::FunctionEndMid() : MidCode(FUN_END_MID)
{
}

void FunctionEndMid::out(ostream& os) const
{
    // TODO
}

vector<MidCode*> midCodes;

map<TokenType, MidOp> MidOpMap = {
        { PLUS, ADD_OP },
        { MINU, SUB_OP },
        { MULT, MUL_OP },
        { DIV,  DIV_OP },
        { LSS,  LSS_OP },
        { LEQ,  LEQ_OP },
        { GRE,  GRE_OP },
        { GEQ,  GEQ_OP },
        { EQL,  EQL_OP },
        { NEQ,  NEQ_OP }
};

int StringVar::count = 0;

set<StringVar*> StringVar::stringVars;

StringVar::StringVar(const string& str) : VarBase(STRING), id(++count)
{
    this->str = string("\"");
    for (char ch : str)
    {
        if (ch == '\\')
        {
            this->str += "\\\\";
        }
        else
        {
            this->str += ch;
        }
    }
    this->str += '"';
    label = string("$string_" + to_string(id));
    stringVars.insert(this);
}

void StringVar::out(ostream& os) const
{
    os << "$string_" + to_string(id);
}

const string& StringVar::getStr() const
{
    return str;
}

const string& StringVar::getLabel() const
{
    return label;
}
