#include "ToMips.h"
#include <set>

#define DEBUG_OUTPUT

static void loadVar(VarBase* var, int reg);

static void storeVar(int reg, VarBase* var);

void addTempVar(set<VarBase*>& vars, VarBase* pVar);

static vector<set<VarBase*>> tempVars;

static int curMiCodeLine;
static unsigned int curStackSize = 0;

static void loadTempVars();

static VarBase* regVars[32];

void globalInitialize(vector<MidCode*>::iterator iterator)
{
    curStackSize = 0;
    curStackSize += 4;
    while (iterator != midCodes.end() && (*iterator)->getType() == ASS_MID)
    {
        auto* assignMid = (AssignMid*)(*iterator);
#ifdef DEBUG_OUTPUT
        assignMid->out(mipsFile);
        mipsFile << endl;
#endif
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
                mipsFile << "subu $" << $t2 << ", $0, $" << $t0 << endl;
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
                mipsFile << "addu $" << $t2 << ", $" << $t0 << ", $" << $t1 << endl;
                break;
            }
            case SUB_OP:
            {
                mipsFile << "subu $" << $t2 << ", $" << $t0 << ", $" << $t1 << endl;
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
        iterator++;
    }
}

void toMips()
{
    loadTempVars();
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
            int size = entry->getArraySize() * 4;
            mipsFile << "$global_" << entry->getName() << "__" << ": .space " << size << endl;
        }
        else
        {
            mipsFile << "$global_" << entry->getName() << "__" << ": .word " << entry->getInitVal() << endl;
        }
    }
    for (auto* pItem : StringVar::stringVars)
    {
        mipsFile << pItem->getLabel() << ": .asciiz \t" << pItem->getStr() << endl;
    }
    mipsFile << ".text" << endl;
    // TO Initialize Global Vars
    auto it = midCodes.begin();
    globalInitialize(it);
    mipsFile << "jal main" << endl;
    mipsFile << "li $v0 10" << endl;
    mipsFile << "syscall" << endl;
    //    curStackSize = TempVar::count * 4;
//    for (const auto& item : getGSymTab()->find("main")->getSymTab()->getSymTab())
//    {
//        auto entry = item.second;
//        if (entry->isFunc()) // Function
//        {
//            // TODO: function generation
//        }
//        else if (entry->getDimension() > 0) // array
//        {
//            // TODO: Array
//        }
//        else
//        {
//            curStackSize += 4;
//            entry->setAddress(curStackSize);
//        }
//    }
    for (; it != midCodes.end(); it++)
    {

        curMiCodeLine = it - midCodes.begin();
        auto midCode = *it;
#ifdef DEBUG_OUTPUT
        midCode->out(mipsFile);
        mipsFile << endl;
#endif
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
                    mipsFile << "subu $" << $t2 << ", $0, $" << $t0 << endl;
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
                    mipsFile << "addu $" << $t2 << ", $" << $t0 << ", $" << $t1 << endl;
                    break;
                }
                case SUB_OP:
                {
                    mipsFile << "subu $" << $t2 << ", $" << $t0 << ", $" << $t1 << endl;
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
            if (writeMid->var == nullptr)
            {
                mipsFile << "li $v0, 11" << endl;
                mipsFile << "la $a0, " << 10 << endl;
            }
            else if (writeMid->isString)
            {
                mipsFile << "li $v0, 4" << endl;
                mipsFile << "la $a0, " << ((StringVar*)(writeMid->var))->getLabel() << endl;
            }
            else
            {
                loadVar(writeMid->var, $a0);
                if (writeMid->isChar)
                {
                    mipsFile << "li $v0, 11" << endl;
                }
                else
                {
                    mipsFile << "li $v0, 1" << endl;
                }
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
        case CALL_MID:
        {
            auto* callMid = (CallMid*)midCode;
            mipsFile << "addiu $sp, $sp, -" << callMid->valTable.size() * 4 + 16 << std::endl;
            curStackSize += callMid->valTable.size() * 4 + 16;
            // Push Params into Stack
            for (int i = 0; i < callMid->valTable.size(); i++)
            {
#ifdef DEBUG_OUTPUT
                mipsFile << "# Pushing param " << i << endl;
#endif
                loadVar(callMid->valTable[i], $a0);
                int to_sp = i * 4;
                mipsFile << "sw $a0, " << to_sp << "($sp)" << std::endl;
            }
            // NO NEED TO RESET REGS
            // UPDATE: NEED TO SAVE $t0 $t1 $t2 $t3 $t4 $t5
            /*VarBase* savedVars[32] = { nullptr };2
            for (int i = $t0; i <= $t5; i++)
            {
                if (regvars[i])
                {
                    savedvars[i] = regvars[i];
                    storevar(i, savedvars[i]);
                }
            }*/
            mipsFile << "jal " << callMid->symTabEntry->getName() << std::endl;
            // RESTORE REGS
            /*for (int i = $t0; i <= $t5; i++)
            {
                if (savedVars[i])
                {
                    loadVar(savedVars[i], i);
                }
            }*/
            mipsFile << "addiu $sp, $sp, " << callMid->valTable.size() * 4 + 16 << std::endl;
            curStackSize -= callMid->valTable.size() * 4 + 16;
            break;
        }
        case RET_MID:
        {
            auto* retMid = (ReturnMid*)midCode;
            if (!retMid->isVoid)
            {
                loadVar(retMid->ret, $t5);
                mipsFile << "move $v0, $" << $t5 << endl;
            }
            mipsFile << "addiu $sp, $sp, " << curStackSize << std::endl;
            mipsFile << "lw $ra, -4($sp)" << std::endl;
            mipsFile << "jr $ra" << std::endl;
            break;
        }
        case BRA_MID:
        {
            auto* braMid = (BranchMid*)midCode;
            loadVar(braMid->left, $t1);
            loadVar(braMid->right, $t2);
            mipsFile << op2Branch[braMid->op] << "$" << $t1 << ", $" << $t2 << ", __LABEL__" << braMid->label->label() << endl;
            break;
        }
        case JMP_MID:
        {
            auto* jumpMid = (JumpMid*)midCode;
            mipsFile << "j __LABEL__" << jumpMid->label->label() << endl;
            break;
        }
        case LAB_MID:
        {
            auto* labMid = (LabelMid*)midCode;
            mipsFile << "__LABEL__" << labMid->label->label() << ":" << endl;
            break;
        }
        case FUN_MID:
        {
            auto* funcMid = (FunctionMid*)midCode;
            for (auto& regVar : regVars)
            {
                regVar = nullptr;
            }
            curStackSize = 0;
            curStackSize += 4;
            mipsFile << funcMid->symTabEntry->getName() << ":" << endl;
            mipsFile << "sw $ra, -4($sp)" << endl;
            for (const auto& e : funcMid->symTabEntry->getSymTab()->getSymTab())
            {
                auto entry = e.second;
                if (entry->isFunc()) // Function
                {
                    // TODO: function generation
                }
                else if (entry->getDimension() > 0) // array
                {
                    curStackSize += entry->getArraySize() * 4;
                    entry->setAddress(curStackSize);
                }
                else
                {
                    curStackSize += 4;
                    entry->setAddress(curStackSize);
                }
            }
            for (auto sub_it = it; (*sub_it)->getType() != FUN_END_MID; sub_it++)
            {
                auto subMidCode = *sub_it;
                bool flag = false;
                VarBase* varBase;
                if (subMidCode->getType() == ASS_MID)
                {
                    flag = true;
                    auto assignMid = (AssignMid*)subMidCode;
                    varBase = assignMid->des;
                }
                //                    else if (subMidCode->getType() == READ_MID)
                //                    {
                //                        flag = true;
                //                        auto readMid = (ReadMid*)subMidCode;
                //                        varBase = readMid->var;
                //                    }
                if (flag && varBase->varType == TEMPVAR)
                {
                    auto temp = (TempVar*)varBase;
                    if (temp->offset == 0)
                    {
                        curStackSize += 4;
                        temp->offset = -curStackSize;
                    }
                }
            }
            mipsFile << "addiu $sp, $sp, -" << curStackSize << endl;
            for (const auto& e : funcMid->symTabEntry->getSymTab()->getSymTab())
            {
                auto entry = e.second;
                if (!entry->isFunc() && entry->getDimension() == 0)
                {
                    unsigned int to_sp = curStackSize - entry->getAddress();
                    mipsFile << "li $v1, " << entry->getInitVal() << endl;
                    mipsFile << "sw $v1, " << to_sp << "($sp)" << endl;
                }
            }
            // LOAD ARGUMENTS
            int argSize = funcMid->symTabEntry->getParamTab()->params.size();
            int argBase = curStackSize;
            for (int i = 0; i < argSize; i++)
            {
                unsigned int to_sp = curStackSize + i * 4;
                mipsFile << "lw $v1, " << to_sp << "($sp)" << endl;
                /*storeVar($v1, new Var(funcMid->symTabEntry->getSymTab()->find(
                    funcMid->symTabEntry->getParamTab()->params[i].GetName()
                )
                ));*/
                for (const auto& e : funcMid->symTabEntry->getSymTab()->getSymTab())
                {
                    auto entry = e.second;
                    if (entry->getName() == funcMid->symTabEntry->getParamTab()->params[i].GetName())
                    {
#ifdef DEBUG_OUTPUT
                        mipsFile << "# Loading param " << i << " : " << funcMid->symTabEntry->getParamTab()->params[i].GetName() << endl;
#endif
                        unsigned int to_sp = curStackSize - entry->getAddress();
                        mipsFile << "sw $v1, " << to_sp << "($sp)" << endl;
                        break;
                    }
                }
            }
            break;
        }
        case FUN_END_MID:
        {
//            // STORE
//            for (int i = 0; i < 32; i++)
//            {
//                if (regVars[i])
//                {
//                    storeVar(i, regVars[i]);
//                }
//            }
            mipsFile << "addiu $sp, $sp, " << curStackSize << endl;
            mipsFile << "lw $ra, -4($sp)" << std::endl;
            mipsFile << "jr $ra" << std::endl;
            break;
        }
        default:
            break;
        }
    }
}

void addTempVar(set<VarBase*>& vars, VarBase* pVar)
{
    if (pVar && pVar->varType == ARRAY)
    {
        auto* arrayVar = (ArrayVar*)pVar;
        pVar = arrayVar->index;
    }
    if (pVar && pVar->varType != TEMPVAR)
    {
        return;
    }
    vars.insert(pVar);
}

void loadVar(VarBase* var, int reg)
{
    regVars[reg] = var;
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
        mipsFile << "lw $" << reg << ", " << to_sp << "($sp)" << endl;
        break;
    }
    case BASEVAR:
    {
        auto baseVar = (Var*)var;
        if (baseVar->symTabEntry->isGlobal())
        {
            mipsFile << "lw $" << reg << ", " << "$global_" << baseVar->symTabEntry->getName() << "__" << endl;
        }
        else
        {
            unsigned int offset = curStackSize - baseVar->symTabEntry->getAddress();
            mipsFile << "lw $" << reg << ", " << offset << "($sp)" << endl;
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
        auto arrayVar = (ArrayVar*)var;
        int offset = -1;
        // offset
        if (arrayVar->index->varType == CONVAR)
        {
            auto constVar = (ConstVar*)(arrayVar->index);
            offset = constVar->value * 4;
        }
        else if (arrayVar->index->varType == TEMPVAR)
        {
            loadVar(arrayVar->index, $t4);
            mipsFile << "sll $" << $t4 << ", $" << $t4 << ", 2" << std::endl;
        }
        else {
            while (1);
        }
        if (arrayVar->symTabEntry->isGlobal())
        {
            if (offset >= 0)
            {
                mipsFile << "lw $" << reg << ", " << "$global_" << arrayVar->symTabEntry->getName() << "__" << "+" << offset << std::endl;
            }
            else
            {
                mipsFile << "lw $" << reg << ", " << "$global_" << arrayVar->symTabEntry->getName() << "__" << "($" << $t4 << ")"
                    << std::endl;
            }
        }
        else
        {
            unsigned int to_sp = curStackSize - arrayVar->symTabEntry->getAddress();
            if (offset >= 0)
            {
                mipsFile << "lw $" << reg << ", " << to_sp + offset << "($sp)" << std::endl;
            }
            else
            {
                mipsFile << "addu $" << $t4 << ", $sp, $" << $t4 << std::endl;
                mipsFile << "lw $" << reg << ", " << to_sp << "($" << $t4 << ")"
                    << std::endl;
            }
        }
        break;
    }
#endif
#ifdef FUNCTIONSUPPORT
    case RETVAR:
    {
        mipsFile << "move $" << reg << ", $v0" << endl;
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
        mipsFile << "sw $" << reg << ", " << offset << "($sp)" << endl;
        break;
    }
    case BASEVAR:
    {
        auto baseVar = (Var*)var;
        if (baseVar->symTabEntry->isGlobal())
        {
            mipsFile << "sw $" << reg << ", " << "$global_" << baseVar->symTabEntry->getName() << "__" << endl;
        }
        else
        {
            unsigned int offset = curStackSize - baseVar->symTabEntry->getAddress();
            mipsFile << "sw $" << reg << ", " << offset << "($sp)" << endl;
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
        auto arrayVar = (ArrayVar*)var;
        int offset = -1;
        if (arrayVar->index->varType == CONVAR)
        {
            auto constVar = (ConstVar*)(arrayVar->index);
            offset = constVar->value * 4;
        }
        else if (arrayVar->index->varType == TEMPVAR)
        {
            loadVar(arrayVar->index, $t4);
            mipsFile << "sll $" << $t4 << ", $" << $t4 << ", 2" << std::endl;
        }
        if (arrayVar->symTabEntry->isGlobal())
        {
            if (offset >= 0)
            {
                mipsFile << "sw $" << reg << ", " << "$global_" << arrayVar->symTabEntry->getName() << "__" << "+" << offset << std::endl;
            }
            else
            {
                mipsFile << "sw $" << reg << ", " << "$global_" << arrayVar->symTabEntry->getName() << "__" << "($" << $t4 << ")"
                    << std::endl;
            }
        }
        else
        {
            unsigned int to_sp = curStackSize - arrayVar->symTabEntry->getAddress();
            if (offset >= 0)
            {
                mipsFile << "sw $" << reg << ", " << to_sp + offset << "($sp)" << std::endl;
            }
            else
            {
                mipsFile << "addu $" << $t4 << ", $sp, $" << $t4 << std::endl;
                mipsFile << "sw $" << reg << ", " << to_sp << "($" << $t4 << ")"
                    << std::endl;
            }
        }
    }
#endif
    default:
    {
        break;
    }
    }
    regVars[reg] = nullptr;
}


static void loadTempVars()
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
        case CALL_MID:
        {
            auto* callMid = (CallMid*)midCode;
            for (auto* var : callMid->valTable)
            {
                addTempVar(tempVars[i], var);
            }
            break;
        }
        case RET_MID:
        {
            auto* retMid = (ReturnMid*)midCode;
            if (!retMid->isVoid)
            {
                addTempVar(tempVars[i], retMid->ret);
            }
            break;
        }
        case BRA_MID:
        {
            auto* branchMid = (BranchMid*)midCode;
            addTempVar(tempVars[i], branchMid->left);
            addTempVar(tempVars[i], branchMid->right);
            break;
        }
        case FUN_MID:
        {
            auto* funcMid = (FunctionMid*)midCode;
            break;
        }
        default:
            break;
        }
    }
}