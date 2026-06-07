// ============================================================
// Lexer.cpp -- Lexical Analyzer Implementation
// ============================================================
// Phase 1: Reads source code character by character.
// Produces a stream of Token structs for the Parser.
// Handles: identifiers, keywords, literals, operators,
// delimiters, whitespace, and comments (// and /* */).
// ============================================================

#include "../include/Lexer.h"

using namespace std;

// constructor
Lexer::Lexer(const string& src)
{
    source = src;
    pos = 0;
    line = 1;
    col = 1;
}

// peek current character
char Lexer::peek()
{
    if ((unsigned)pos >= source.size())
        return '\0';

    return source[pos];
}

// move forward
char Lexer::advance()
{
    if ((unsigned)pos >= source.size())
        return '\0';

    char c = source[pos];
    pos++;

    if (c == '\n')
    {
        line++;
        col = 1;
    }
    else
    {
        col++;
    }

    return c;
}

// skip spaces + comments
void Lexer::skipWhitespace()
{
    while (true)
    {
        char c = peek();

        // spaces, tabs, newlines
        if (c == ' ' || c == '\t' || c == '\n')
        {
            advance();
            continue;
        }

        // comments
        if (c == '/' && (unsigned)pos + 1 < source.size())
        {
            // single line comment //
            if (source[(unsigned)pos + 1] == '/')
            {
                advance(); advance();
                while (peek() != '\n' && peek() != '\0')
                    advance();
                continue;
            }

            // multi-line comment /* */
            if (source[(unsigned)pos + 1] == '*')
            {
                advance(); advance();

                while (peek() != '\0')
                {
                    if (peek() == '*' && (unsigned)pos + 1 < source.size() && source[(unsigned)pos + 1] == '/')
                    {
                        advance(); advance();
                        break;
                    }
                    advance();
                }
                continue;
            }
        }

        break;
    }
}

// identifiers + keywords
Token Lexer::readIdentifier()
{
    string lexeme;
    int startLine = line;
    int startCol = col;

    while (true)
    {
        char c = peek();

        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_')
        {
            lexeme += advance();
        }
        else break;
    }

    TokenType type = ID;

    if      (lexeme == "int")     type = INT;
    else if (lexeme == "float")   type = FLOAT;
    else if (lexeme == "bool")    type = BOOL;
    else if (lexeme == "void")    type = VOID;
    else if (lexeme == "if")      type = IF;
    else if (lexeme == "else")    type = ELSE;
    else if (lexeme == "while")   type = WHILE;
    else if (lexeme == "for")     type = FOR;
    else if (lexeme == "return")  type = RETURN;
    else if (lexeme == "input")   type = INPUT;
    else if (lexeme == "output")  type = OUTPUT;
    else if (lexeme == "true")    type = TRUE;
    else if (lexeme == "false")   type = FALSE;

    return {type, lexeme, startLine, startCol};
}

// numbers
Token Lexer::readNumber()
{
    string lexeme;
    int startLine = line;
    int startCol = col;
    bool isFloat = false;

    while (peek() >= '0' && peek() <= '9')
        lexeme += advance();

    if (peek() == '.')
    {
        isFloat = true;
        lexeme += advance();

        while (peek() >= '0' && peek() <= '9')
            lexeme += advance();
    }

    TokenType type = isFloat ? FLOAT_VAL : INT_VAL;

    return {type, lexeme, startLine, startCol};
}

// operators
Token Lexer::readOperator()
{
    int startLine = line;
    int startCol = col;

    char c = advance();
    string lexeme(1, c);
    TokenType type = ILLEGAL;

    switch (c)
    {
        case '+': type = PLUS; break;
        case '-': type = MINUS; break;
        case '*': type = STAR; break;
        case '/': type = SLASH; break;

        case '(': type = LPAREN; break;
        case ')': type = RPAREN; break;
        case '{': type = LBRACE; break;
        case '}': type = RBRACE; break;

        case ';': type = SEMICOLON; break;
        case ',': type = COMMA; break;

        case '=':
            if (peek() == '=') { lexeme += advance(); type = EQ; }
            else type = ASSIGN;
            break;

        case '!':
            if (peek() == '=') { lexeme += advance(); type = NEQ; }
            else type = NOT;
            break;

        case '<':
            if (peek() == '=') { lexeme += advance(); type = LEQ; }
            else type = LT;
            break;

        case '>':
            if (peek() == '=') { lexeme += advance(); type = GEQ; }
            else type = GT;
            break;

        case '&':
            if (peek() == '&') { lexeme += advance(); type = AND; }
            break;

        case '|':
            if (peek() == '|') { lexeme += advance(); type = OR; }
            break;
    }

    return {type, lexeme, startLine, startCol};
}

// main function
Token Lexer::nextToken()
{
    skipWhitespace();

    char c = peek();

    if (c == '\0')
        return {END_OF_FILE, "", line, col};

    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
         c == '_')
        return readIdentifier();

    if (c >= '0' && c <= '9')
        return readNumber();

    return readOperator();
}