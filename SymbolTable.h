//
// Created by chao4 on 2020/10/18 0018.
//

#ifndef COMPILER_0_SYMBOLTABLE_H
#define COMPILER_0_SYMBOLTABLE_H

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

#endif //COMPILER_0_SYMBOLTABLE_H
