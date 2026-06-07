// ============================================================
// SemanticAnalyzer.h -- Type Checker and Semantic Analysis
// ============================================================
// Phase 5: Walks the AST and performs:
//   - Variable declaration checking
//   - Type compatibility checking
//   - Return type matching
//   - Operator type validation
// ============================================================

#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include <iostream>
#include <string>
#include "AST.h"
#include "ErrorHandler.h"
#include "SymbolTable.h"

using namespace std;

class SemanticAnalyzer {
private:
    SymbolTable symTable;         // Tracks declared variables and their types
    ErrorHandler* errors;         // Error reporting
    TokenType currentReturnType;  // Expected return type of current function

    void visitFunctionDecl(FunctionDecl* func);
    void visitStmt(Stmt* stmt);
    TokenType visitExpr(Expr* expr);   // Returns the type of the expression

    bool isNumeric(TokenType t) const;          // Is type int or float?
    bool canAssign(TokenType dest, TokenType src) const;  // Can src be assigned to dest?

public:
    SemanticAnalyzer(ErrorHandler* err);

    // Run semantic analysis on the entire program
    void analyze(Program* program);
};

#endif
