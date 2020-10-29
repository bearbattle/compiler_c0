#include "SymbolTable.h"
#include "Utility.h"
#include "Error.h"


SymTabEntry::SymTabEntry(const string& name, BaseType baseType, int line)
{
    this->_name = tolower(name);
    this->_baseType = baseType;
    this->_isConst = false;
    this->_isFunc = false;
    this->_symTab = nullptr;
    this->_dimension = 0;
    this->_paramTab = nullptr;
    this->_line = line;
    this->_initVal = 0;
}

const string& SymTabEntry::getName() const
{
    return _name;
}

BaseType SymTabEntry::getBaseType() const
{
    return _baseType;
}

bool SymTabEntry::isFunc() const
{
    return _isFunc;
}

void SymTabEntry::setFunc(bool isFunc)
{
    _isFunc = isFunc;
    if (_isFunc && _paramTab == nullptr)
    {
        _paramTab = new ParamTab();
    }
}

bool SymTabEntry::isConst() const
{
    return _isConst;
}

void SymTabEntry::setConst(bool isConst)
{
    _isConst = isConst;
}

int SymTabEntry::getDimension() const
{
    return _dimension;
}

void SymTabEntry::setDimension(int dimension)
{
    _dimension = dimension;
}

SymTab* SymTabEntry::getSymTab() const
{
    return _symTab;
}

void SymTabEntry::setSymTab(SymTab* symTab)
{
    _symTab = symTab;
}

ParamTab* SymTabEntry::getParamTab() const
{
    return _paramTab;
}

void SymTabEntry::setParamTab(ParamTab* paramTab)
{
    _paramTab = paramTab;
}

int SymTabEntry::getInitVal() const
{
    return _initVal;
}

void SymTabEntry::setInitVal(int initVal)
{
    SymTabEntry::_initVal = initVal;
}

int SymTabEntry::getLength(int dimension) const
{
    return _length[dimension - 1];
}

void SymTabEntry::setLength(int dimension, int length)
{
    _length[dimension - 1] = length;
}

int SymTabEntry::getLine() const
{
    return _line;
}

void initSymTabs()
{
    SymTabs.push_back(new SymTab());
}

SymTab*& getGSymTab()
{
    return SymTabs.back();
}

deque<SymTab*> SymTabs;

SymTabEntry* SymTab::find(const string& str)
{
    string lowercase = tolower(str);
    int i = this->layer;
    while (i >= 0)
    {
        auto item = _symTab.find(lowercase);
        if (item != _symTab.end())
        {
            return _symTab[lowercase];
        }
        i--;
    }
    return nullptr;
}

bool SymTab::exist(const string& str, bool curLayer)
{
    if (curLayer)
    {
        auto item = _symTab.find(tolower(str));
        if (item != _symTab.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return find(str) != nullptr;
    }
}

SymTab::SymTab()
{
    _symTab = map<string, SymTabEntry*>();
    layer = SymTabs.size();
    SymTabs.push_back(this);
}

void SymTab::addEntry(const string& name, SymTabEntry* entry, bool override)
{
    bool duplicate = exist(name, true);
    if (override || !duplicate)
    {
        _symTab[name] = entry;
    }
    if (!override && duplicate)
    {
        errList.emplace_back(b, entry->getLine());
        // TODO: Check the line of Token()
    }
}

void ParamTab::addParam(BaseType type, const string& name)
{
    params.emplace_back(type, name);
}

bool ParamTab::fitParam(int order, BaseType type)
{
    return order < params.size() &&
        params[order].getType() == type;
}

Param::Param(BaseType type, const string& name) : type(type), name(name)
{
    string lowercase;
    this->type = type;
    this->name = tolower(name);
}

BaseType Param::getType() const
{
    return type;
}
