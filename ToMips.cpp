#include "ToMips.h"
#include <set>

static void loadVar(VarBase* var, int reg);

static void storeVar(int reg, VarBase* var);

void addTempVar(set<VarBase*>& tempVars, VarBase* pVar);

static vector<set<VarBase*>> tempVars;

static int curMiCodeLine;
static unsigned int curStackSize = 0;

void toMips()
{
    tempVars.resize(midCodes.size());
    for (int i = 0; i < midCodes.size(); i++)
    {
        if (i > 0)
        {
            tempVars[i] = tempVars[i - 1];
        }
        auto midCode = midCodes[i];
        switch (midCode->getType())
        {
        case ASS_MID:
        {
            auto* assignMid = (AssignMid*)midCode;
            addTempVar(tempVars[i], assignMid->left);
            if (assignMid->right != nullptr)
            {
                addTempVar(tempVars[i], assignMid->right);
            }
            addTempVar(tempVars[i], assignMid->des);
            break;
        }
        case WRITE_MID:
        {
            auto* writeMid = (WriteMid*)midCode;
            addTempVar(tempVars[i], writeMid->var);
            break;
        }
        case READ_MID:
        {
            auto* readMid = (ReadMid*)midCode;
            //            addTempVar(tempVars[i], readMid->var);
            break;
        }
        default:
            break;
        }

    }
    mipsFile << ".data" << std::endl;
    for (const auto& e : getGSymTab()->getSymTab())
    {
        auto entry = e.second;
        if (entry->isFunc()) // Function
        {
            // TODO: function generation
        }
        else if (entry->getDimension() > 0) // array
        {
            // TODO: Array
        }
        else
        {
            mipsFile << entry->getName() << ": .word " << entry->getInitVal() << endl;
        }
    }
    for (auto* pItem : StringVar::stringVars)
    {
        mipsFile << pItem->getLabel() << ": .asciiz \t" << pItem->getStr() << endl;
    }
    mipsFile << ".text" << endl;
    //    mipsFile << "jal main" << endl;
    //    mipsFile << "li $v0 10" << endl;
    //    mipsFile << "syscall" << endl;
    //    curStackSize = TempVar::count * 4;
    for (const auto& item : getGSymTab()->find("main")->getSymTab()->getSymTab())
    {
        auto entry = item.second;
        if (entry->isFunc()) // Function
        {
            // TODO: function generation
        }
        else if (entry->getDimension() > 0) // array
        {
            // TODO: Array
        }
        else
        {
            curStackSize += 4;
            entry->setAddress(curStackSize);
        }
    }
    for (auto it = midCodes.begin(); it != midCodes.end(); it++)
    {
        curMiCodeLine = it - midCodes.begin();
        auto midCode = *it;
        switch (midCode->getType())
        {
        case ASS_MID:
        {
            auto* assignMid = (AssignMid*)midCode;
            if (assignMid->right == nullptr)
            {
                loadVar(assignMid->left, $t0);
                switch (assignMid->op)
                {
                case ADD_OP:
                {
                    mipsFile << "move $" << $t2 << ", $" << $t0 << endl;
                    break;
                }
                case SUB_OP:
                {
                    mipsFile << "sub $" << $t2 << ", $0, $" << $t0 << endl;
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
            else
            {
                loadVar(assignMid->left, $t0);
                loadVar(assignMid->right, $t1);
                switch (assignMid->op)
                {
                case ADD_OP:
                {
                    mipsFile << "add $" << $t2 << ", $" << $t0 << ", $" << $t1 << endl;
                    break;
                }
                case SUB_OP:
                {
                    mipsFile << "sub $" << $t2 << ", $" << $t0 << ", $" << $t1 << endl;
                    break;
                }
                case MUL_OP:
                {
                    mipsFile << "mul $" << $t2 << ", $" << $t0 << ", $" << $t1 << endl;
                    break;
                }
                case DIV_OP:
                {
                    mipsFile << "div $" << $t0 << ", $" << $t1 << std::endl;
                    mipsFile << "mflo $" << $t2 << std::endl;
                    break;
                }
                default:
                    // TODO: Implementation
                    break;
                }
            }
            storeVar($t2, assignMid->des);
            break;
        }
        case WRITE_MID:
        {
            auto* writeMid = (WriteMid*)midCode;
            if (writeMid->isString)
            {
                mipsFile << "li $v0, 4" << endl;
                mipsFile << "la $a0, " << ((StringVar*)(writeMid->var))->getLabel() << endl;
            }
            else
            {
                if (writeMid->isChar)
                {
                    mipsFile << "li $v0, 11" << endl;
                }
                else
                {
                    mipsFile << "li $v0, 1" << endl;
                }
                loadVar(writeMid->var, $a0);
            }
            mipsFile << "syscall" << endl;
            break;
        }
        case READ_MID:
        {
            auto* readMid = (ReadMid*)midCode;
            if (readMid->var->symTabEntry->getBaseType() == CHAR)
            {
                mipsFile << "li $v0, 12" << std::endl;
            }
            else
            {
                mipsFile << "li $v0, 5" << std::endl;
            }
            mipsFile << "syscall" << std::endl;
            mipsFile << "move $" << $t3 << ",$v0" << std::endl;
            storeVar($t3, readMid->var);
            break;
        }
        default:
            break;
        }
    }
    mipsFile << "li $a0, 10" << endl;
    mipsFile << "syscall" << endl;
}

void addTempVar(set<VarBase*>& tempVars, VarBase* pVar)
{
    if (pVar->varType != TEMPVAR)
    {
        return;
    }
    tempVars.insert(pVar);
}

void loadVar(VarBase* var, int reg)
{
    switch (var->varType)
    {
    case TEMPVAR:
    {
        auto tempVar = (TempVar*)var;
        if (tempVar->offset == 0)
        {
            while (1);
            //wrong;
        }
        long long to_sp = curStackSize + tempVar->offset;
        mipsFile << "lw $" << reg << ", -" << to_sp << "($sp)" << endl;
        break;
    }
    case BASEVAR:
    {
        auto baseVar = (Var*)var;
        if (baseVar->symTabEntry->isGlobal())
        {
            mipsFile << "lw $" << reg << ", " << baseVar->symTabEntry->getName() << endl;
        }
        else
        {
            unsigned int offset = curStackSize - baseVar->symTabEntry->getAddress();
            mipsFile << "lw $" << reg << ", -" << offset << "($sp)" << endl;
        }
        break;
    }
    case CONVAR:
    {
        auto constVar = (ConstVar*)var;
        mipsFile << "li $" << reg << ", " << constVar->value << endl;
        break;
    }
#ifdef ARRAYSUPPORT
    case ARRAY:
    {
        // TODO
    }
#endif
    default:
    {
        break;
    }
    }
}

void storeVar(int reg, VarBase* var)
{
    switch (var->varType)
    {
    case TEMPVAR:
    {
        auto tempVar = (TempVar*)var;
        if (tempVar->offset == 0)
        {
            while (1);
            //wrong;
        }
        long long offset = curStackSize + tempVar->offset;
        mipsFile << "sw $" << reg << ", -" << offset << "($sp)" << endl;
        break;
    }
    case BASEVAR:
    {
        auto baseVar = (Var*)var;
        if (baseVar->symTabEntry->isGlobal())
        {
            mipsFile << "sw $" << reg << ", " << baseVar->symTabEntry->getName() << endl;
        }
        else
        {
            unsigned int offset = curStackSize - baseVar->symTabEntry->getAddress();
            mipsFile << "sw $" << reg << ", -" << offset << "($sp)" << endl;
        }
        break;
    }
    case CONVAR:
    {
        // ERROR
        break;
    }
#ifdef ARRAYSUPPORT
    case ARRAY:
    {
        // TODO
    }
#endif
    default:
    {
        break;
    }
    }
}
