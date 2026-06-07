// ============================================================
// index.cpp -- Main Driver / Compiler Pipeline
// ============================================================
// Orchestrates all compiler phases:
//   File I/O → Lexer → Parser → SemanticAnalyzer → CodeGenerator
// Supports .c source files as input.
// Usage: ./compiler source.c
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "include/Lexer.h"
#include "include/Parser.h"
#include "include/SemanticAnalyzer.h"
#include "include/CodeGenerator.h"
#include "include/ErrorHandler.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <source-file>" << endl;
        return 1;
    }

    ifstream file(argv[1]);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file '" << argv[1] << "'" << endl;
        return 1;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    string source = buffer.str();

    ErrorHandler errors;

    // Phase 1: Lexical Analysis (Scanner)
    // Reads source text, produces a stream of tokens
    cout << "=== Phase 1: Lexical Analysis ===" << endl;
    Lexer lexer(source);

    Token tokens[MAX_TOKENS];
    int tokenCount = 0;

    bool lexicalError = false;

    while (tokenCount < MAX_TOKENS)
    {
        Token t = lexer.nextToken();
        tokens[tokenCount++] = t;
        if (t.type == END_OF_FILE || t.type == ILLEGAL)
            break;
    }

    for (int i = 0; i < tokenCount; i++)
    {
        if (tokens[i].type == ILLEGAL)
        {
            errors.addError(tokens[i].line, tokens[i].col, "Lexical",
                "Illegal character '" + tokens[i].lexeme + "'");
            lexicalError = true;
        }
    }

    if (errors.hasErrors())
    {
        errors.printErrors();
        errors.clear();
    }

    if (lexicalError)
    {
        cerr << "Lexical analysis failed. Aborting." << endl;
        return 1;
    }

    // Print token stream
    cout << "\n=== Token Stream ===\n" << endl;
    for (int i = 0; i < tokenCount; i++)
    {
        Token t = tokens[i];

        cout << "[" << t.line << ":" << t.col << "] ";

        switch (t.type)
        {
            case INT:        cout << "INT";        break;
            case FLOAT:      cout << "FLOAT";      break;
            case BOOL:       cout << "BOOL";       break;
            case VOID:       cout << "VOID";       break;
            case IF:         cout << "IF";         break;
            case ELSE:       cout << "ELSE";       break;
            case WHILE:      cout << "WHILE";      break;
            case FOR:        cout << "FOR";        break;
            case RETURN:     cout << "RETURN";     break;
            case INPUT:      cout << "INPUT";      break;
            case OUTPUT:     cout << "OUTPUT";     break;
            case TRUE:       cout << "TRUE";       break;
            case FALSE:      cout << "FALSE";      break;
            case PLUS:       cout << "PLUS";       break;
            case MINUS:      cout << "MINUS";      break;
            case STAR:       cout << "STAR";       break;
            case SLASH:      cout << "SLASH";      break;
            case ASSIGN:     cout << "ASSIGN";     break;
            case EQ:         cout << "EQ";         break;
            case NEQ:        cout << "NEQ";        break;
            case LT:         cout << "LT";         break;
            case GT:         cout << "GT";         break;
            case LEQ:        cout << "LEQ";        break;
            case GEQ:        cout << "GEQ";        break;
            case AND:        cout << "AND";        break;
            case OR:         cout << "OR";         break;
            case NOT:        cout << "NOT";        break;
            case LPAREN:     cout << "LPAREN";     break;
            case RPAREN:     cout << "RPAREN";     break;
            case LBRACE:     cout << "LBRACE";     break;
            case RBRACE:     cout << "RBRACE";     break;
            case SEMICOLON:  cout << "SEMICOLON";  break;
            case COMMA:      cout << "COMMA";      break;
            case ID:         cout << "ID";         break;
            case INT_VAL:    cout << "INT_VAL";    break;
            case FLOAT_VAL:  cout << "FLOAT_VAL";  break;
            case END_OF_FILE: cout << "EOF";       break;
            case ILLEGAL:    cout << "ILLEGAL";    break;
            default:         cout << "UNKNOWN";    break;
        }

        cout << "  '" << t.lexeme << "'" << endl;
    }

    // Phase 2: Syntax Analysis (Parser + AST construction)
    // Builds AST from token stream via recursive descent
    cout << "\n=== Phase 2: Syntax Analysis ===" << endl;
    Parser parser;
    parser.setTokens(tokens, tokenCount, &errors);
    Program* program = parser.parse();

    if (errors.hasErrors())
    {
        errors.printErrors();
        cout << "\nSyntax analysis failed." << endl;
        errors.clear();
        delete program;
        return 1;
    }

    cout << "Parsing successful." << endl;

    // Phase 3: Semantic Analysis (Type checking + Symbol Table)
    // Walks AST, resolves variables, validates types
    cout << "\n=== Phase 3: Semantic Analysis ===" << endl;
    SemanticAnalyzer analyzer(&errors);
    analyzer.analyze(program);

    if (errors.hasErrors())
    {
        errors.printErrors();
        cout << "\nSemantic analysis failed." << endl;
        errors.clear();
        delete program;
        return 1;
    }

    cout << "Semantic analysis successful." << endl;

    // Phase 4: Code Generation (Three Address Code)
    // Walks AST, emits TAC instructions
    cout << "\n=== Phase 4: Code Generation ===" << endl;
    CodeGenerator codeGen;
    codeGen.generate(program);
    codeGen.printCode();

    cout << "\n\nCompilation Successful!" << endl;

    delete program;
    return 0;
}
