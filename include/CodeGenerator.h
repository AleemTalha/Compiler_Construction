// ============================================================
// CodeGenerator.h -- Three Address Code (TAC) Generator
// ============================================================
// Phase 7: Walks the AST and emits Three Address Code
// instructions. TAC is a low-level IR where each instruction
// has at most one operator and three operands.
// ============================================================

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <iostream>
#include <string>
#include "AST.h"

using namespace std;

const int MAX_INSTRUCTIONS = 1000;

// A single Three Address Code instruction
struct TACInst {
    string op;       // Operation: ADD, SUB, MUL, DIV, ASSIGN, JUMP, etc.
    string arg1;     // First operand
    string arg2;     // Second operand (for binary ops)
    string result;   // Destination (temporary or variable name)
    string label;    // Label name (for LABEL, IF_FALSE, JUMP targets)
};

class CodeGenerator {
private:
    TACInst instructions[MAX_INSTRUCTIONS];   // Generated TAC instructions
    int instCount;                            // Number of instructions generated
    int tempCount;                            // Counter for temporary variables (t0, t1, ...)
    int labelCount;                           // Counter for labels (L0, L1, ...)

    string newTemp();     // Generate a new temporary variable name
    string newLabel();    // Generate a new label name
    string genExpr(Expr* expr);               // Generate code for an expression, return temp
    void genStmt(Stmt* stmt);                 // Generate code for a statement
    void genFunctionDecl(FunctionDecl* func); // Generate code for a function

public:
    CodeGenerator();

    // Generate TAC for the entire program
    void generate(Program* program);

    // Print the generated TAC instructions
    void printCode() const;
};

#endif
