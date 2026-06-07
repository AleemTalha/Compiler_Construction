// ============================================================
// Parser.h -- Recursive Descent Parser
// ============================================================
// Phase 3: Parses a stream of tokens into an Abstract Syntax
// Tree using recursive descent (one function per grammar rule).
// Implements panic-mode error recovery.
// ============================================================

#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include "Lexer.h"
#include "AST.h"
#include "ErrorHandler.h"

using namespace std;

const int MAX_TOKENS = 1000;    // Maximum tokens in a source file

class Parser {
private:
    Token tokens[MAX_TOKENS];    // Token stream (fixed array)
    int tokenCount;              // Number of tokens
    int pos;                     // Current position in token stream
    ErrorHandler* errors;        // Error reporting

    // Token stream helpers
    Token peek() const;          // Look at current token
    Token advance();             // Move to next token, return previous
    Token previous() const;      // Get previously consumed token
    bool isAtEnd() const;        // Check if end of token stream
    Token expect(TokenType type, const string& errorMsg);  // Consume and validate
    bool match(TokenType type);  // Check and consume if matches
    void sync();                 // Panic-mode recovery: skip to ; or }

    // Grammar functions (one per BNF rule)
    Program* parseProgram();
    FunctionDecl* parseFunctionDecl();
    void parseParamList(FunctionDecl* func);
    Stmt* parseStmt();
    Stmt* parseDeclStmt();
    Stmt* parseAssignStmt(const string& name);
    Stmt* parseIfStmt();
    Stmt* parseWhileStmt();
    Stmt* parseForStmt();
    Stmt* parseReturnStmt();
    Stmt* parseInputStmt();
    Stmt* parseOutputStmt();
    Stmt* parseBlockStmt();

    // Expression parsing (precedence climbing)
    Expr* parseExpr();
    Expr* parseOrExpr();
    Expr* parseAndExpr();
    Expr* parseEqExpr();
    Expr* parseRelExpr();
    Expr* parseAddExpr();
    Expr* parseMulExpr();
    Expr* parseUnaryExpr();
    Expr* parsePrimaryExpr();
    Expr* parseFunCallExpr(const string& name);

    TokenType parseType();       // Parse a type keyword (int, float, bool, void)

public:
    Parser();

    // Initialize the parser with a token stream
    void setTokens(const Token* tkns, int count, ErrorHandler* err);

    // Parse the token stream into a Program AST
    Program* parse();
};

#endif
