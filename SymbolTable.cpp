//
// Created by chao4 on 2020/10/18 0018.
//

#include "SymbolTable.h"

SymbolTableEntry::SymbolTableEntry(string name, BaseType baseType) {
    this->_name = name;
    this->_baseType = baseType;
}

const string &SymbolTableEntry::getName() const {
    return _name;
}

void SymbolTableEntry::setName(const string &name) {
    _name = name;
}

BaseType SymbolTableEntry::getBaseType() const {
    return _baseType;
}

void SymbolTableEntry::setBaseType(BaseType baseType) {
    _baseType = baseType;
}

bool SymbolTableEntry::isFunc() const {
    return _isFunc;
}

void SymbolTableEntry::setFunc(bool isFunc) {
    _isFunc = isFunc;
}

bool SymbolTableEntry::isConst() const {
    return _isConst;
}

void SymbolTableEntry::setConst(bool isConst) {
    _isConst = isConst;
}

int SymbolTableEntry::getDimension() const {
    return _dimension;
}

void SymbolTableEntry::setDimension(int dimension) {
    _dimension = dimension;
}

void initSymTabs() {
    SymTabs.emplace_back();
}

map<string, SymbolTableEntry *> getGSymTab() {
    return SymTabs.back();
}

deque<map<string, SymbolTableEntry *>> SymTabs;