//
// Created by chao4 on 2020/11/14 0014.
//

#include "MidCode.h"

#define DEBUG_OUTPUT


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
    offset = 0;
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


ArrayVar::ArrayVar(SymTabEntry* symTabEntry, VarBase* subscript, VarBase* subscript1) : VarBase(ARRAY),
symTabEntry(symTabEntry)
{
    if (subscript1 == nullptr)// One Dimension Array
    {
        if (subscript->varType == CONVAR)
        {
            index = subscript;
        }
        else
        {
            auto* t1 = new TempVar();
            midCodes.push_back(new AssignMid(ADD_OP, subscript, t1));
            index = t1;
        }
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

void ArrayVar::out(ostream& os) const
{
    os << symTabEntry->getName() << "[";
    index->out(os);
    os << "]";
}

int BaseLabel::count = 0;

BaseLabel::BaseLabel(bool inc)
{
    if (inc)
    {
        id = count++;
    }
    else
    {
        id = count;
    }
}

string BaseLabel::label() const
{
    return string("$label_") + to_string(id);
}

MidCode::MidCode(MidType type) : type(type)
{
}

MidType MidCode::getType() const
{
    return type;
}

static map<MidOp, string> op2Str = { // NOLINT
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
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
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
#ifdef DEBUG_OUTPUT
        os << "# ";
#endif
        os << "push ";
        item->out(os);
        os << endl;
    }
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
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
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
    os << "return";
    if (!isVoid)
    {
        os << ' ';
        ret->out(os);
    }
}


static map<MidOp, MidOp> reverseOp = { // NOLINT
        { LSS_OP, GEQ_OP },
        { LEQ_OP, GRE_OP },
        { GRE_OP, LEQ_OP },
        { GEQ_OP, LSS_OP },
        { EQL_OP, NEQ_OP },
        { NEQ_OP, EQL_OP },
};

BranchMid::BranchMid(MidOp op, VarBase* left, VarBase* right, BaseLabel* label,
    bool reverse) : MidCode(BRA_MID),
    left(left),
    right(right),
    label(label)
{
    if (reverse)
    {
        this->op = reverseOp[op];
    }
    else
    {
        this->op = op;
    }
}

map<MidOp, string> op2Branch = { // NOLINT
        { LSS_OP, "blt " },
        { LEQ_OP, "ble " },
        { GRE_OP, "bgt " },
        { GEQ_OP, "bge " },
        { EQL_OP, "beq " },
        { NEQ_OP, "bne " }
};

void BranchMid::out(ostream& os) const
{
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
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
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
    os << "j " << label->label() << ":";
}

FunctionMid::FunctionMid(SymTabEntry* symTabEntry) : MidCode(FUN_MID), symTabEntry(symTabEntry)
{
}

void FunctionMid::out(ostream& os) const
{
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
    bool first = true;
    os << symTabEntry->getName() << "(";
    for (auto item : symTabEntry->getParamTab()->params)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            os << ", ";
        }
        os << item;
    }
    os << ")" << endl;
    for (const auto& item : symTabEntry->getSymTab()->getSymTab())
    {
#ifdef DEBUG_OUTPUT
        os << "# ";
#endif
        if (item.second->isConst())
        {
            os << "local var const " << item.first << " = " << item.second->getInitVal() << std::endl;
        }
        else if (item.second->getDimension() > 0) // array
        {
            os << "local var array " << item.first << "[" << item.second->getArraySize() << "]" << std::endl;
        }
        else
        {
            os << "local var " << item.first << " = " << item.second->getInitVal() << std::endl;
        }
    }
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
    os << "{";
}

ReadMid::ReadMid(Var* var) : MidCode(READ_MID), var(var)
{
}

void ReadMid::out(ostream& os) const
{
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
    os << "read ";
    var->out(os);
}

WriteMid::WriteMid() : MidCode(WRITE_MID), var(nullptr), isChar(false), isString(false)
{
}

WriteMid::WriteMid(VarBase* var, bool isChar) : MidCode(WRITE_MID), var(var), isChar(isChar)
{
    isString = typeid(*var) == typeid(StringVar);
}

void WriteMid::out(ostream& os) const
{
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
    os << "write ";
    if (var == nullptr)
    {
        os << "newline";
        return;
    }
    else if (typeid(*var) == typeid(StringVar))
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
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
    os << "}";
}

vector<MidCode*> midCodes;

map<TokenType, MidOp> MidOpMap = { // NOLINT
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

IfLabel::IfLabel() : BaseLabel(), elseLabel(new ElseLabel), endLabel(new EndLabel)
{

}

LoopLabel::LoopLabel() : BaseLabel(), beginLabel(new BeginLabel), endLabel(new EndLabel)
{
}

ElseLabel::ElseLabel() : BaseLabel(false)
{
}

string ElseLabel::label() const
{
    return string("$else_" + to_string(id));
}

string IfLabel::label() const
{
    return string("$if_" + to_string(id));
}

EndLabel::EndLabel() : BaseLabel(false)
{

}

string EndLabel::label() const
{
    return string("$end_" + to_string(id));
}

LabelMid::LabelMid(BaseLabel* label) : MidCode(LAB_MID), label(label)
{
}

void LabelMid::out(ostream& os) const
{
#ifdef DEBUG_OUTPUT
    os << "# ";
#endif
    os << label->label() << ":";
}

BeginLabel::BeginLabel() : BaseLabel(false)
{

}

string BeginLabel::label() const
{
    return string("$loop_begin_" + to_string(id));
}

CaseLabel::CaseLabel(SwitchLabel* switchLabel, long id) : BaseLabel(false),
switchLabel(switchLabel),
id(id)
{
}

string CaseLabel::label() const
{
    return string(switchLabel->label() + "_case_" + to_string(id));
}

SwitchLabel::SwitchLabel(VarBase* caseVar) : BaseLabel(), caseVar(caseVar), endLabel(new EndLabel)
{
    caseLabels.push(new CaseLabel(this, caseLabels.size()));
}

CaseLabel* SwitchLabel::curCaseLabel()
{
    return caseLabels.top();
}

CaseLabel* SwitchLabel::nextCaseLabel()
{
    caseLabels.push(new CaseLabel(this, caseLabels.size()));
    return caseLabels.top();
}

string SwitchLabel::label() const
{
    return string("$switch_" + to_string(id));
}
