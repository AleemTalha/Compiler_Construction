// ============================================================
// SymbolTable.cpp -- Symbol Table Implementation
// ============================================================
// Uses a flat array of Symbol structs indexed by scope level.
// Scope 0 = global, each pushScope() increments level.
// ============================================================

#include "../include/SymbolTable.h"

SymbolTable::SymbolTable() : symbolCount(0), currentLevel(0) {}

void SymbolTable::pushScope()
{
    currentLevel++;
}

void SymbolTable::popScope()
{
    if (currentLevel > 0) currentLevel--;
}

bool SymbolTable::insert(const Symbol& sym, ErrorHandler* errors)
{
    // Check for redeclaration in current scope
    for (int i = 0; i < symbolCount; i++)
    {
        if (symbols[i].name == sym.name && symbols[i].scopeLevel == currentLevel)
        {
            if (errors)
            {
                errors->addError(sym.line, 1, "Semantic",
                    "Redeclaration of variable '" + sym.name + "'");
            }
            return false;
        }
    }

    // Add to symbol table
    if (symbolCount < MAX_SYMBOLS)
    {
        symbols[symbolCount] = sym;
        symbolCount++;
        return true;
    }

    return false;
}

Symbol* SymbolTable::lookup(const string& name)
{
    // Search from most recent to oldest, respecting scope levels
    for (int i = symbolCount - 1; i >= 0; i--)
    {
        if (symbols[i].name == name && symbols[i].scopeLevel <= currentLevel)
            return &symbols[i];
    }
    return nullptr;
}

Symbol* SymbolTable::lookupCurrentScope(const string& name)
{
    // Search only in current scope level
    for (int i = symbolCount - 1; i >= 0; i--)
    {
        if (symbols[i].name == name && symbols[i].scopeLevel == currentLevel)
            return &symbols[i];
    }
    return nullptr;
}

int SymbolTable::getCurrentLevel() const
{
    return currentLevel;
}
