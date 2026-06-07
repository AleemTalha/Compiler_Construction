// ============================================================
// SymbolTable.h -- Nested Scope Symbol Table
// ============================================================
// Phase 4: Manages variable declarations with nested block
// scoping. Uses a flat array with scope levels.
// ============================================================

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <iostream>
#include <string>
#include "Lexer.h"
#include "ErrorHandler.h"

using namespace std;

const int MAX_SYMBOLS = 200;

// Information about a declared variable
struct Symbol {
    string name;         // Variable name
    TokenType type;      // int, float, bool, or void
    int line;            // Line where declared
    int scopeLevel;      // Which scope it belongs to
};

class SymbolTable {
private:
    Symbol symbols[MAX_SYMBOLS];   // Flat array of all symbols
    int symbolCount;               // Total symbols stored
    int currentLevel;              // Current scope depth

public:
    SymbolTable();

    // Open a new scope (enter a block)
    void pushScope();

    // Close the current scope (exit a block)
    void popScope();

    // Add a symbol to the current scope.
    // Returns false if symbol already exists in current scope.
    bool insert(const Symbol& sym, ErrorHandler* errors);

    // Find a symbol by name, searching from innermost scope outward
    Symbol* lookup(const string& name);

    // Find a symbol only in the current scope
    Symbol* lookupCurrentScope(const string& name);

    int getCurrentLevel() const;
};

#endif
