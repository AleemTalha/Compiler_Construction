// ============================================================
// AST.h -- Abstract Syntax Tree Node Definitions
// ============================================================
// Phase 2: Defines the AST node hierarchy used to represent
// the structure of valid programs after parsing.
// ============================================================

#ifndef AST_H
#define AST_H

#include <string>
#include "Lexer.h"

using namespace std;

// Maximum sizes for fixed arrays (no vectors used)
const int MAX_ARGS = 10;          // Max function call arguments
const int MAX_PARAMS = 10;        // Max function parameters
const int MAX_FUNCTIONS = 50;     // Max functions in a program

// Forward declarations
class Expr;
class Stmt;

// A function parameter (type + name pair)
struct Param {
    TokenType type;     // int, float, bool, or void
    string name;        // Parameter name
    Param() : type(INT), name("") {}
    Param(TokenType t, const string& n) : type(t), name(n) {}
};

// Base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() {}
};

// ==================== Expression Nodes ====================

class Expr : public ASTNode {
public:
    virtual ~Expr() {}
};

// Integer literal (e.g., 42)
class IntLiteral : public Expr {
public:
    int value;
    IntLiteral(int v) : value(v) {}
};

// Float literal (e.g., 3.14)
class FloatLiteral : public Expr {
public:
    float value;
    FloatLiteral(float v) : value(v) {}
};

// Boolean literal (true or false)
class BoolLiteral : public Expr {
public:
    bool value;
    BoolLiteral(bool v) : value(v) {}
};

// Variable reference (e.g., x)
class Identifier : public Expr {
public:
    string name;
    Identifier(const string& n) : name(n) {}
};

// Unary expression (e.g., -x, !flag)
class UnaryExpr : public Expr {
public:
    TokenType op;        // NOT or MINUS
    Expr* operand;
    UnaryExpr(TokenType o, Expr* e) : op(o), operand(e) {}
    ~UnaryExpr() { delete operand; }
};

// Binary expression (e.g., a + b, x > y)
class BinaryExpr : public Expr {
public:
    Expr* left;
    TokenType op;
    Expr* right;
    BinaryExpr(Expr* l, TokenType o, Expr* r) : left(l), op(o), right(r) {}
    ~BinaryExpr() { delete left; delete right; }
};

// Function call expression (e.g., factorial(x))
class FunCallExpr : public Expr {
public:
    string name;
    Expr* args[MAX_ARGS];     // Fixed-size array of arguments
    int argCount;

    FunCallExpr(const string& n) : name(n), argCount(0) {
        for (int i = 0; i < MAX_ARGS; i++) args[i] = nullptr;
    }

    void addArg(Expr* e) {
        if (argCount < MAX_ARGS) args[argCount++] = e;
    }

    ~FunCallExpr() {
        for (int i = 0; i < argCount; i++) delete args[i];
    }
};

// ==================== Statement Nodes ====================

// Base class for all statements.
// Statements are linked via the 'next' pointer (linked list).
class Stmt : public ASTNode {
public:
    Stmt* next;          // Pointer to next statement in sequence
    Stmt() : next(nullptr) {}
    virtual ~Stmt() {}
};

// Variable declaration (e.g., int x = 5;)
class DeclStmt : public Stmt {
public:
    TokenType type;      // Declared type (int, float, bool)
    string name;         // Variable name
    Expr* init;          // Optional initializer expression
    int line;            // Line number for error reporting

    DeclStmt(TokenType t, const string& n, int ln, Expr* i = nullptr)
        : type(t), name(n), init(i), line(ln) {}
    ~DeclStmt() { delete init; }
};

// Assignment (e.g., x = 10;)
class AssignStmt : public Stmt {
public:
    string name;
    Expr* value;
    AssignStmt(const string& n, Expr* v) : name(n), value(v) {}
    ~AssignStmt() { delete value; }
};

// If-else statement
class IfStmt : public Stmt {
public:
    Expr* cond;
    Stmt* thenStmt;
    Stmt* elseStmt;       // nullptr if no else clause
    IfStmt(Expr* c, Stmt* t, Stmt* e = nullptr) : cond(c), thenStmt(t), elseStmt(e) {}
    ~IfStmt() { delete cond; delete thenStmt; delete elseStmt; }
};

// While loop
class WhileStmt : public Stmt {
public:
    Expr* cond;
    Stmt* body;
    WhileStmt(Expr* c, Stmt* b) : cond(c), body(b) {}
    ~WhileStmt() { delete cond; delete body; }
};

// For loop
class ForStmt : public Stmt {
public:
    Stmt* init;
    Expr* cond;
    Stmt* update;
    Stmt* body;
    ForStmt(Stmt* i, Expr* c, Stmt* u, Stmt* b) : init(i), cond(c), update(u), body(b) {}
    ~ForStmt() { delete init; delete cond; delete update; delete body; }
};

// Return statement
class ReturnStmt : public Stmt {
public:
    Expr* value;
    ReturnStmt(Expr* v = nullptr) : value(v) {}
    ~ReturnStmt() { delete value; }
};

// Input statement (e.g., input(x);)
class InputStmt : public Stmt {
public:
    string name;
    InputStmt(const string& n) : name(n) {}
};

// Output statement (e.g., output(x);)
class OutputStmt : public Stmt {
public:
    Expr* expr;
    OutputStmt(Expr* e) : expr(e) {}
    ~OutputStmt() { delete expr; }
};

// Block statement (a sequence of statements inside { })
class BlockStmt : public Stmt {
public:
    Stmt* first;          // First statement in the block (linked via next)

    BlockStmt(Stmt* f = nullptr) : first(f) {}

    ~BlockStmt() {
        // Walk the linked list and delete each statement
        Stmt* cur = first;
        while (cur) {
            Stmt* temp = cur;
            cur = cur->next;
            delete temp;
        }
    }
};

// ==================== Structure Nodes ====================

// Function declaration
class FunctionDecl : public ASTNode {
public:
    TokenType returnType;            // Return type
    string name;                     // Function name
    Param params[MAX_PARAMS];        // Parameter list
    int paramCount;                  // Number of parameters
    BlockStmt* body;                 // Function body

    FunctionDecl(TokenType rt, const string& n, BlockStmt* b)
        : returnType(rt), name(n), paramCount(0), body(b) {}

    void addParam(const Param& p) {
        if (paramCount < MAX_PARAMS) params[paramCount++] = p;
    }

    ~FunctionDecl() { delete body; }
};

// Program (top-level node containing all functions)
class Program : public ASTNode {
public:
    FunctionDecl* functions[MAX_FUNCTIONS];
    int funcCount;

    Program() : funcCount(0) {
        for (int i = 0; i < MAX_FUNCTIONS; i++) functions[i] = nullptr;
    }

    void addFunction(FunctionDecl* f) {
        if (funcCount < MAX_FUNCTIONS) functions[funcCount++] = f;
    }

    ~Program() {
        for (int i = 0; i < funcCount; i++) delete functions[i];
    }
};

#endif
