// ============================================================
// Lexer.h -- Token types, Token struct, and Lexer class
// ============================================================
// Phase 1: Lexical Analyzer
// Reads source code character by character and produces a
// stream of tokens recognized by the language grammar.
// ============================================================

#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>

using namespace std;

// All token types recognized by the compiler
enum TokenType {
    // Data types
    INT, FLOAT, BOOL, VOID,

    // Keywords
    IF, ELSE, WHILE, FOR, RETURN,

    // Arithmetic operators
    PLUS, MINUS, STAR, SLASH,

    // I/O keywords
    INPUT, OUTPUT, TRUE, FALSE,

    // Assignment and comparison operators
    ASSIGN, EQ, NEQ, LT, GT, LEQ, GEQ, AND, OR, NOT,

    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE,

    // Statement terminators and separators
    SEMICOLON, COMMA,

    // Identifier and literal values
    ID, INT_VAL, FLOAT_VAL,

    // Special tokens
    END_OF_FILE, ILLEGAL
};

// Represents a single token from the source code
struct Token {
    TokenType type;   // What kind of token
    string lexeme;    // The actual text matched
    int line;         // Line number in source
    int col;          // Column number in source
};

// Lexer: scans source text and returns tokens one at a time
class Lexer {
private:
    string source;    // The entire source code
    int pos;          // Current position in source
    int line;         // Current line number
    int col;          // Current column number

public:
    Lexer(const string& src);   // Constructor takes source code
    Token nextToken();          // Get the next token from source

private:
    char peek();                // Look at current character without consuming
    char advance();             // Consume current character and move forward
    void skipWhitespace();      // Skip spaces, tabs, newlines, and comments
    Token readIdentifier();     // Read an identifier or keyword
    Token readNumber();         // Read an integer or float literal
    Token readOperator();       // Read an operator or delimiter
};

#endif
