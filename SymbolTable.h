#ifndef COMPILER_0_SYMBOLTABLE_H
#define COMPILER_0_SYMBOLTABLE_H

#include <string>
#include <map>
#include <vector>
#include <deque>

using namespace std;

enum BaseType
{
    UNDEF, INT, CHAR, VOID
};

static map<BaseType, string> baseTypeOut = { // NOLINT
        { UNDEF, "UNDEFINED" },
        { INT,   "int" },
        { CHAR,  "char" },
        { VOID,  "void" },
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
    bool _isGlobal;
    unsigned int _address = 0;
public:
    SymTabEntry(const string& name, BaseType baseType, int line, bool isGlobal = false);

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

    bool isGlobal() const;

    unsigned int getAddress() const;

    void setAddress(unsigned int address);

    int getArraySize() const;
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

    const map<string, SymTabEntry*>& getSymTab() const;
};

class Param
{
private:
    BaseType type;
    string name;
public:
    Param(BaseType type, const string& name);

    BaseType getType() const;

    friend ostream& operator<<(ostream& out, Param& A);

    const string& GetName() const;
};

class ParamTab
{
public:
    vector<Param> params;

    void addParam(BaseType type, const string& name);

    void checkParams(const vector<BaseType>& valTable, int line);
};

extern deque<SymTab*> SymTabs;

void initSymTabs();

SymTab*& getGSymTab();

#endif //COMPILER_0_SYMBOLTABLE_H
